#pragma once

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 5039 4668 4710 4711)
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
            record_new_instance("CTOR");
        }

        checker(const std::string& context)
        {
            record_new_instance(context);
        }

        checker(const checker& rhs)
        {
            check_name();
            auto rhs_id = get_instance_id(&rhs);
            *out_ << fmt::format("COPY CTOR {}, this={}, id={} | rhs={}, rhs_id={}, count before={}",
                class_name_, cast_format(this), current_id_, cast_format(&rhs), rhs_id, instances_.size()) << std::endl;
            record_new_instance_id("COPY CTOR");
        }

        ~checker()
        {
            check_name();
            auto id = get_instance_id(this);
            if (id != ENTRY_NOT_FOUND)
            {
                instances_.erase(this);
                *out_ << fmt::format("DTOR {}, this={}, id={}, count after={}", class_name_, cast_format(this), id, instances_.size()) << std::endl;
                if (control_ids_.contains(id))
                    callback_(id, class_name_, "DTOR");

            }
            else
                *out_ << fmt::format("ERROR: instance not found, this={}", cast_format(this)) << std::endl;

            if (instances_.size() <= displayThreshold_)
            {
                for (const auto& v : instances_)
                    *out_ << fmt::format("=     ptr={} id={}", cast_format(v.first), v.second) << std::endl;
            }
        }

        static size_t addControlIds(const std::vector<long>& list)
        {
            *out_ << fmt::format("## Set controlIds, size: {} (Module='{}')", list.size(), get_module_name()) << std::endl;
            for (auto id : list)
                control_ids_.insert(id);
            return control_ids_.size();
        }

        static size_t setCallback(checker_callback_t callback)
        {
            *out_ << fmt::format("## Set callback: {} (Module='{}')", (void*)callback, get_module_name()) << std::endl;
            callback_ = callback;
            return 0;
        }

    private:

        void record_new_instance(const std::string& context)
        {
            check_name();
            *out_ << fmt::format("CTOR {} {}, this={}, id={}, count before={}", class_name_, context, cast_format(this), current_id_, instances_.size()) << std::endl;
            record_new_instance_id(context);
        }

        void record_new_instance_id(const std::string& context)
        {
            auto id = get_instance_id(this);
            if (id == ENTRY_NOT_FOUND)
                instances_[this] = current_id_;
            else
                *out_ << fmt::format("ERROR: instance already exists, pointer={} id1={} id2={}", cast_format(this), id, current_id_) << std::endl;
            if (control_ids_.contains(current_id_))
                callback_(current_id_, class_name_, context);
            ++current_id_;
        }

        static void display_controlled(long id, const std::string& name, const std::string& eventName)
        {
            *out_ << fmt::format("Controlled instance: {} for class {} Event={}", id, name, eventName) << std::endl;
        }

        void check_name()
        {
            if (class_name_.empty())
            {
                class_name_ = typeid(T).name();
                *out_ << fmt::format("==== Start Recording instances for class '{}' (Module='{}')", class_name_, get_module_name()) << std::endl;
            }
        }

        id_type get_instance_id(const checker* ptr)
        {
            auto it = instances_.find(ptr);
            if (it != instances_.end())
                return it->second;
            return ENTRY_NOT_FOUND;
        }

        const void* cast_format(const checker* ptr)
        {
            return (const void*)ptr;
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
