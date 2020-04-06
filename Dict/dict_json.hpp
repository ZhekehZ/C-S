#pragma once

#include "dict.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

#include <sstream>

namespace utils {

    void write_as_json(const dict_t &dict, rapidjson::Writer<rapidjson::StringBuffer> &writer) {
        writer.StartObject();
        for (const auto&[key, value] : dict) {
            writer.Key(key.c_str());
            if (const auto *sub_dict = whatever_cast<dict_t>(&value)) {
                write_as_json(*sub_dict, writer);
            } else {
                auto[isInt, i] = value.as_int();
                if (isInt) {
                    writer.Int(i);
                    continue;
                }
                auto[isDouble, d] = value.as_double();
                if (isDouble) {
                    writer.Double(d);
                    continue;
                }
                auto[isBool, b] = value.as_bool();
                if (isBool) {
                    writer.Bool(b);
                    continue;
                }
                auto[isString, s] = value.as_string();
                if (isString) {
                    writer.String(s.data());
                    continue;
                }
            }
        }
        writer.EndObject();
    }


    void save_to_json(std::ostream &os, const dict_t &dict) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        write_as_json(dict, writer);
        os << buffer.GetString();
    }


    bool read_from_json(decltype(rapidjson::Document().GetObject()) obj, dict_t &dict) {
        for (auto&[key, value] : obj) {
            const std::string keyName = key.GetString();
            if (value.IsUint()) {
                put(dict, keyName, static_cast<int>(value.GetUint()));
            }
            if (value.IsInt()) {
                put(dict, keyName, value.GetInt());
                continue;
            }
            if (value.IsDouble()) {
                put(dict, keyName, value.GetDouble());
                continue;
            }
            if (value.IsBool()) {
                put(dict, keyName, value.GetBool());
                continue;
            }
            if (value.IsString()) {
                put(dict, keyName, value.GetString());
                continue;
            }
            if (value.IsObject()) {
                dict_t subdict;
                read_from_json(value.GetObject(), subdict);
                put(dict, key.GetString(), std::move(subdict));
                continue;
            }
            return false;
        }
        return true;
    }

    bool load_from_json(std::istream &is, dict_t &dict) {
        std::string str(std::istreambuf_iterator<char>(is), {});
        rapidjson::StringStream stream(str.c_str());
        rapidjson::Document document;
        document.ParseStream(stream);

        if (!document.IsObject()) {
            return false;
        }

        return read_from_json(document.GetObject(), dict);
    }

}
