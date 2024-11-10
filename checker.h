#pragma once
#include <map>
#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <set>
#include <vector>

class checker_common
{
public:
    static void setThreshold(int threshold)
    {
        displayThreshold_ = threshold;
    }

    static void setOutput(const std::string& fileName)
    {
        out_ = new std::ofstream(fileName);
        if (!out_->good())
            throw std::runtime_error("Error: cannot open file " + fileName);
    }

protected:
    static inline int displayThreshold_ = 3;
    static inline std::ostream* out_ = &std::cout;
};

using checker_callback_t = void (*) (long id, const std::string& class_name, const std::string& event);

template<typename T>
class checker : public checker_common
{
public:
    using id_type = long;

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
        *out_ << std::format("COPY CTOR {}, this={}, id={} | rhs={}, rhs_id={}", class_name_, cast_format(this), current_id_, cast_format(&rhs), rhs_id) << std::endl;
        record_new_instance_id("COPY CTOR");
    }

    ~checker()
    {
        check_name();
        auto it = instances_.find(this);
        if (it != instances_.end())
        {
            *out_ << std::format("DTOR {}, this={}, id={}", class_name_, cast_format(this), it->second) << std::endl;
            if (control_ids_.contains(it->second))
                callback_(it->second, class_name_, "DTOR");
            instances_.erase(it);
        }
        else
            *out_ << std::format("ERROR: instance not found, this={}", cast_format(this)) << std::endl;

        if (instances_.size() <= displayThreshold_)
        {
            *out_ << "= Remaining instances for "<< class_name_ <<  ":" << instances_.size() << std::endl;
            for (const auto& v : instances_)
                *out_ << std::format("=     ptr={} id={}", cast_format(v.first), v.second) << std::endl;
        }
    }

    static void addControlIds(const std::vector<long>& list)
    {
        for (auto id : list)
            control_ids_.insert(id);
    }

    static void setCallback(checker_callback_t callback)
    {
        callback_ = callback;
    }

private:

    void record_new_instance(const std::string& context)
    {
        check_name();
        *out_ << std::format("CTOR {} {}, this={}, id={}", class_name_, context, cast_format(this), current_id_) << std::endl;
        record_new_instance_id(context);
    }

    void record_new_instance_id(const std::string& context)
    {
        auto it = instances_.find(this);
        if (it == instances_.end())
            instances_[this] = current_id_;
        else
            *out_ << std::format("ERROR: instance already exists, pointer={} id1={} id2={}", cast_format(this), it->second, current_id_) << std::endl;
        if (control_ids_.contains(current_id_))
            callback_(current_id_, class_name_, context);
        ++current_id_;
    }

    static void display_controlled(long id, const std::string& name, const std::string& event)
    {
        *out_ << "Controlled instance: " << id << " for class " << name << " Event=" << event << std::endl;
    }

    void check_name()
    {
        if (class_name_.empty())
        {
            class_name_ = typeid(T).name();
            *out_ << "==== Start Recording instances for class " << class_name_ << std::endl;
        }
    }

    id_type get_instance_id(const checker* ptr)
    {
        auto it = instances_.find(ptr);
        if (it != instances_.end())
            return it->second;
        return -1;
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
