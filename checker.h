#pragma once

#include <map>
#include <iostream>
#include <format>
#include <string>

template<typename T>
class checker
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
        auto it = instances_.find(this);
        if (it == instances_.end())
            instances_[this] = current_id_;
        else
            std::cout << std::format("ERROR: instance already exists, pointer={} id1={} id2={}", (void*)this, it->second, current_id_) << std::endl;
        ++current_id_;
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

        if (instances_.size()<3)
        {
            std::cout << "Remaining instances:" << instances_.size() << std::endl;
        }

    }

private:

    void record_new_instance(const std::string& context)
    {
        check_name();
        std::cout << std::format("CTOR {} {}, this={}, id={}", class_name_, context, (void*)this, current_id_) << std::endl;
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

    long get_instance_id(const checker* ptr)
    {
        auto it = instances_.find((void*)ptr);
        if (it != instances_.end())
            return it->second;
        return -1;
    }

    static inline long current_id_ = 0;
    static inline std::map<void*, long> instances_;
    static inline std::string class_name_;
};

