#pragma once

#include "whatever.hpp"
#include "rapidjson/writer.h"

#include <unordered_map>
#include <string>
#include <map>


namespace utils {

    using dict_t = whatever::dict_t;


    class no_key_exception : std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class invalid_type_exception : std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };


    inline bool contains(const dict_t& dict, const std::string& key) {
        return dict.find(key) != dict.end();
    }

    inline bool remove(dict_t& dict, const std::string& key) {
        return dict.erase(key) > 0;
    }

    inline bool empty(const dict_t& dict) {
        return dict.empty();
    }

    inline void clear(dict_t& dict) {
        dict.clear();
    }

    template <typename T>
    bool put(dict_t& dict, const std::string& key, T&& value) {
        return dict.emplace(key, whatever(std::forward<T>(value))).second;
    }

    template <typename T>
    const T* get_ptr(const dict_t& dict, const std::string& key) {
        auto elem = dict.find(key);
        if (elem == dict.end()) {
            return nullptr;
        }
        return whatever_cast<T>(&elem->second);
    }

    template <typename T>
    const T& get(const dict_t& dict, const std::string& key) {
        auto elem = dict.find(key);
        if (elem == dict.end()) {
            throw no_key_exception("[get_ptr] (key = " + key + ")");
        }
        const T* ptr = whatever_cast<T>(&elem->second);
        if (ptr == nullptr) {
            throw invalid_type_exception("[get]");
        }
        return *ptr;
    }

    template <typename T>
    T* get_ptr(dict_t& dict, const std::string& key) {
        return const_cast<T*>(get_ptr<T>(std::as_const(dict), key));
    }

    template <typename T>
    T& get(dict_t& dict, const std::string& key) {
        return const_cast<T&>(get<T>(std::as_const(dict), key));
    }

    inline bool is_dict(const dict_t& dict, const std::string& key) {
        return get_ptr<dict_t>(dict, key) != nullptr;
    }

}
