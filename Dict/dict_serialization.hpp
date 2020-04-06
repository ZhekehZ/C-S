#pragma once

#include "dict.hpp"

#include <vector>

namespace utils {

    inline void write(dict_t& dict, const dict_t& other) {
        dict = other;
    }

    inline void read(const dict_t& other, dict_t& dict) {
        dict = other;
    }

    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    void write(dict_t& dict, const std::vector<T>& value) {
        clear(dict);
        for (size_t i = 0; i < value.size(); ++i) {
            put(dict, std::to_string(i), value[i]);
        }
    }

    template <typename T, std::enable_if_t<!std::is_arithmetic_v<T>, bool> = 0>
    void write(dict_t& dict, const std::vector<T>& value) {
        clear(dict);
        for (size_t i = 0; i < value.size(); ++i) {
            dict_t new_dict;
            write(new_dict, value[i]);
            put(dict, std::to_string(i), new_dict);
        }
    }

    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = 0>
    void write(dict_t& dict, const std::map<std::string, T>& value) {
        clear(dict);
        for (const auto& [key, val] : value) {
            put(dict, key, val);
        }
    }

    template <typename T, std::enable_if_t<!std::is_arithmetic_v<T>, bool> = 0>
    void write(dict_t& dict, const std::map<std::string, T>& value) {
        clear(dict);
        for (const auto& [key, val] : value) {
            dict_t new_dict;
            write(new_dict, val);
            put(dict, key, new_dict);
        }
    }


    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = 0>
    void read(const dict_t& dict, std::vector<T>& out) {
        for (size_t idx = 0; ; ++idx) {
            const std::string key = std::to_string(idx);
            if (!contains(dict, key)) {
                break;
            }
            out.emplace_back(get<T>(dict, key));
        }
    }

    template <typename T, std::enable_if_t<!std::is_arithmetic_v<T>, bool> = 0>
    void read(const dict_t& dict, std::vector<T>& out) {
        for (size_t idx = 0; ; ++idx) {
            const std::string key = std::to_string(idx);
            if (!contains(dict, key)) {
                break;
            }
            T elem = {};
            read(get<dict_t>(dict, key), elem);
            out.emplace_back(elem);
        }
    }

    template <typename T>
    typename std::enable_if_t<std::is_arithmetic_v<T>>
    read(const dict_t& dict, std::map<std::string, T>& out) {
        for (const auto& [key, val] : dict) {
            out.emplace(key, whatever_cast<T>(val));
        }
    }

    template <typename T>
    typename std::enable_if_t<!std::is_arithmetic_v<T>>
    read(const dict_t& dict, std::map<std::string, T>& out) {
        for (const auto& [key, val] : dict) {
            T elem = {};
            read(whatever_cast<dict_t>(val), elem);
            out.emplace(key, elem);
        }
    }

}