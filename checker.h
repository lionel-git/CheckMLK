#pragma once

#include <map>
#include <iostream>
#include <format>
#include <string>

class checker_common
{
public:
    static void setThreshold(int threshold)
    {
        displayThreshold_ = threshold;
    }
protected:
    static inline int displayThreshold_ = 3;
};

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
        auto rhs_id = get_instance_id(&rhs);
        std::cout << std::format("COPY CTOR {}, this={}, id={} | rhs={}, rhs_id={}", class_name_, (void*)this, current_id_, (void*)&rhs, rhs_id) << std::endl;
        record_new_instance_id();
    }

    ~checker()
    {
        check_name();
        auto it = instances_.find(this);
        if (it != instances_.end())
        {
            std::cout << std::format("DTOR {}, this={}, id={}", class_name_, (void*)this, it->second) << std::endl;
            instances_.erase(it);
        }
        else
            std::cout << std::format("ERROR: instance not found, this={}", (void*)this) << std::endl;

        if (instances_.size() <= displayThreshold_)
        {
            std::cout << "= Remaining instances:" << instances_.size() << std::endl;
            for (const auto& v : instances_)
                std::cout << std::format("=     ptr={} id={}", (void*)v.first, v.second) << std::endl;
        }
    }

private:
    using id_type = long;

    void record_new_instance(const std::string& context)
    {
        check_name();
        std::cout << std::format("CTOR {} {}, this={}, id={}", class_name_, context, (void*)this, current_id_) << std::endl;
        record_new_instance_id();
    }

    void record_new_instance_id()
    {
        auto it = instances_.find(this);
        if (it == instances_.end())
            instances_[this] = current_id_;
        else
            std::cout << std::format("ERROR: instance already exists, pointer={} id1={} id2={}", (void*)this, it->second, current_id_) << std::endl;
        ++current_id_;
    }

    void check_name()
    {
        if (class_name_.empty())
            class_name_ = typeid(T).name();
    }

    id_type get_instance_id(const checker* ptr)
    {
        auto it = instances_.find(ptr);
        if (it != instances_.end())
            return it->second;
        return -1;
    }

    static inline id_type current_id_ = 0;
    static inline std::map<const checker<T>*, id_type> instances_;
    static inline std::string class_name_;
};

