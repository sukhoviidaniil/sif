/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-17
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_IO_FROM_JSON_H
#define RENDER_ENGINE_IO_FROM_JSON_H

#include <filesystem>
#include <fstream>

#include "json.hpp"
#include "sif/diagnostics/Logger.h"

namespace sif::io {

    inline nlohmann::json read_json(std::istream& is) {
        nlohmann::json data;
        is >> data;
        return data;
    }

    inline nlohmann::json get_json_data(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            const std::string error = "File not found: " + path.string() + "!\n";
            LOG(error);
            throw std::runtime_error(error);
        }

        file.seekg(0, std::ios::end);
        if (file.tellg() == 0) {
            const std::string error = "File is empty: " + path.string();
            LOG(error);
            throw std::runtime_error(error);
        }
        file.seekg(0);

        return read_json(file);
    }

    inline nlohmann::json get_json_data(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            const std::string error = "File not found: " + filename + "!\n";
            LOG(error);
            throw std::runtime_error(error);
        }
        return read_json(file);
    }

    inline void invalid_parameter(const std::string &path, const std::string &object, const std::string &name) {
        const std::string error = "File: " + path+ ", in " + object +  ", parameter " + name + " missing or invalid;";
        LOG(error);
        throw std::runtime_error(error);
    }

    template<class T>
    T get_checked(
    const nlohmann::json &j,
    const std::string &key,
    const T &default_value
    ) {
        // Key not found -> return default
        if (!j.contains(key))
            return default_value;

        const auto& value = j.at(key);

        // If the JSON value type is correct -> return it
        try {
            return value.get<T>();
        }
        catch (const std::exception& e) {
            LOG(e.what());
            throw;
        }
    }

    template<class T>
    T get_checked(
        const nlohmann::json &j,
        const std::string &key
        ) {
        // Key not found -> err
        if (!j.contains(key)) {
            const std::string error = "nlohmann::json " + key + " parameter missing or invalid;";
            LOG(error);
            throw std::runtime_error(error);
        }
        // If the JSON value type is correct -> return it
        try {
            return j.at(key).get<T>();
        } catch (const std::exception& e) {
            LOG(e.what());
            throw;
        }
    }

    template<class T>
    T get_checked(
    const nlohmann::json &j
    ) {
        // If the JSON value type is correct -> return it
        try {
            return j.get<T>();
        } catch (const std::exception& e) {
            LOG(e.what());
            throw;
        }
    }
}


#endif //RENDER_ENGINE_IO_FROM_JSON_H