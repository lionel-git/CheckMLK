#pragma once

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4365 5039 4668 4710 4711 4820)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <version>
#if defined(__cpp_lib_format)
#include <format>
namespace fmt = std;
#else
#include <fmt/format.h>
#endif

#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <vector>
#include <sstream>

namespace mlk
{
    using id_type = long;

    constexpr id_type ENTRY_NOT_FOUND = -123456789;

    class checker_common
    {
    public:
        static size_t setThreshold(size_t threshold)
        {
            displayThreshold_ = threshold;
            *out_ << fmt::format("## Set Threshold: {} (Module='{}')", displayThreshold_, get_module_name()) << std::endl;
            return displayThreshold_;
        }

        static size_t setOutput(const std::string& fileName)
        {
            out_ = new std::ofstream(fileName);
            if (!out_->good())
                throw std::runtime_error(fmt::format("Cannot open file: '{}'", fileName));
            *out_ << fmt::format("## Set output: {} (Module='{}')", fileName, get_module_name()) << std::endl;
            return 0;
        }

    protected:
        static std::string get_module_name()
        {
            return get_module_name(&displayThreshold_);
        }

        static std::string get_module_name(void* ptr)
        {
#ifdef _WIN32
            char path[MAX_PATH];
            HMODULE hModule = NULL;
            if ((GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)ptr, &hModule) != 0)
                && GetModuleFileNameA(hModule, path, MAX_PATH) != 0)
                return path;
#else
            Dl_info info;
            info.dli_fname = nullptr;
            if (dladdr(ptr, &info) != 0 && info.dli_fname != nullptr)
                return info.dli_fname;
#endif
            return "N/A";
        }

        static inline size_t displayThreshold_ = 3;
        static inline std::ostream* out_ = &std::cout;
    };

    using checker_callback_t = void (*) (long id, const std::string& class_name, const std::string& event);

    template<typename T>
    class checker : public checker_common
    {
    public:
        checker()
        {
            record_new_instance("");
        }

        checker(const std::string& context)
        {
            record_new_instance(context);
        }

        checker(const checker& rhs)
        {
            check_name();
            id_type rhs_id;
            get_instance_id(&rhs, rhs_id);
            *out_ << fmt::format("COPY CTOR {} | {}",
                format_ptr_id(true, "this", this, "id", current_id_, ""), format_ptr_id(false, "rhs", &rhs, "rhs_id", rhs_id, "count before")) << std::endl;
            record_new_instance_id("COPY CTOR");
        }

        ~checker()
        {
            check_name();
            id_type id;
            if (get_instance_id(this, id))
            {
                instances_.erase(this);
                *out_ << fmt::format("DTOR {}", format_ptr_id(true, "this", this, "id", id, "count after")) << std::endl;
                if (control_ids_.contains(id))
                    callback_(id, class_name_, "DTOR");
            }
            else
                *out_ << fmt::format("ERROR: instance not found, this={}", format_ptr(this)) << std::endl;

            if (instances_.size() <= displayThreshold_)
            {
                for (const auto& v : instances_)
                    *out_ << fmt::format("=     {}", format_ptr_id(false, "ptr", v.first, "id", v.second, "")) << std::endl;
            }
        }

        static size_t addControlIds(const std::vector<long>& list)
        {
            check_name();
            *out_ << fmt::format("## Set controlIds for {}, size={} (Module='{}')", class_name_, list.size(), get_module_name()) << std::endl;
            for (auto id : list)
                control_ids_.insert(id);
            return control_ids_.size();
        }

        static size_t setCallback(checker_callback_t callback)
        {
            check_name();
            *out_ << fmt::format("## Set callback for {}: {} (Module='{}')", class_name_, (void*)callback, get_module_name()) << std::endl;
            callback_ = callback;
            return 0;
        }

        static std::string get_statistics()
        {
            std::ostringstream oss;
            if (!class_name_.empty())
            {
                oss << "Class Name: " << class_name_ << std::endl;
                oss << " Current id: " << current_id_ << std::endl;
                oss << " Instances: " << instances_.size();
            }
            else
            {
                oss << "No event recorded for type: " << typeid(T).name();
            }
            return oss.str();
        }

    private:

        void record_new_instance(const std::string& context)
        {
            check_name();
            std::string context_str = context.empty() ? "" : fmt::format("({})", context);
            *out_ << fmt::format("CTOR{} {}", context_str, format_ptr_id(true, "this", this, "id", current_id_, "count before")) << std::endl;
            record_new_instance_id(context);
        }

        void record_new_instance_id(const std::string& context)
        {
            id_type id;
            if (!get_instance_id(this, id))
                instances_[this] = current_id_;
            else
                *out_ << fmt::format("ERROR: instance already exists, pointer={} id1={} id2={}", format_ptr(this), format_id(id), format_id(current_id_)) << std::endl;
            if (control_ids_.contains(current_id_))
            {
                std::string ctx = "CTOR";
                if (!context.empty())
                    ctx += " [" + context + "]";
                callback_(current_id_, class_name_, ctx);
            }
            ++current_id_;
        }

        static void display_controlled(long id, const std::string& name, const std::string& eventName)
        {
            *out_ << fmt::format("Controlled instance: {} for class {} Event={}", id, name, eventName) << std::endl;
        }

        static void check_name()
        {
            if (class_name_.empty())
            {
                class_name_ = typeid(T).name();
                if (class_name_.starts_with("class "))
                    class_name_ = class_name_.substr(6);
                else if (class_name_.starts_with("struct "))
                    class_name_ = class_name_.substr(7);
                auto pos = class_name_.find_first_not_of("0123456789");
                if (pos != std::string::npos)
                    class_name_ = class_name_.substr(pos);
                class_name_ = "[" + class_name_ + "]";
                *out_ << fmt::format("==== Start Recording instances for class '{}' (Module='{}')", class_name_, get_module_name()) << std::endl;
            }
        }

        bool get_instance_id(const checker* ptr, id_type& id)
        {
            auto it = instances_.find(ptr);
            if (it != instances_.end())
            {
                id = it->second;
                return true;
            }
            id = ENTRY_NOT_FOUND;
            return false;
        }

        const void* format_ptr(const checker* ptr)
        {
            return (const void*)ptr;
        }

        const std::string format_id(id_type id)
        {
            return id == ENTRY_NOT_FOUND ? "ENTRY_NOT_FOUND" : fmt::format("{}", id);
        }

        std::string format_ptr_id(bool show_classname, 
            const std::string& msg_ptr, const checker* ptr, 
            const std::string& msg_id, id_type id,
            const std::string& count_msg)
        {
            std::string classname_str = show_classname ? fmt::format("{}, ", class_name_) : "";
            std::string count_msg_str = count_msg.empty() ? "" : fmt::format(", {}={}", count_msg, instances_.size());
            return fmt::format("{}{}={}, {}={}{}", classname_str, msg_ptr, format_ptr(ptr), msg_id, format_id(id), count_msg_str);
        }

        static inline id_type current_id_ = 0;
        static inline std::map<const checker<T>*, id_type> instances_;
        static inline std::string class_name_;
        static inline std::set<id_type> control_ids_;
        static inline checker_callback_t callback_ = display_controlled;
    };

} // namespace mlk

#ifdef _WIN32
#pragma warning(pop)
#endif
