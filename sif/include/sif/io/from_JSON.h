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
#include <system_error>

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

    inline void invalid_parameter(const std::string& path, const std::string& object, const std::string& name) {
        const std::string error = "File: " + path + ", in " + object + ", parameter " + name + " missing or invalid;";
        LOG(error);
        throw std::runtime_error(error);
    }

    template<class T>
    T get_checked(const nlohmann::json& j, const std::string& key, const T& default_value) {
        // Key not found -> return default
        if (!j.contains(key))
            return default_value;

        const auto& value = j.at(key);

        // If the JSON value type is correct -> return it
        try {
            return value.get<T>();
        } catch (const std::exception& e) {
            LOG(e.what());
            throw;
        }
    }

    template<class T>
    T get_checked(const nlohmann::json& j, const std::string& key) {
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
    T get_checked(const nlohmann::json& j) {
        // If the JSON value type is correct -> return it
        try {
            return j.get<T>();
        } catch (const std::exception& e) {
            LOG(e.what());
            throw;
        }
    }

    /**
     * @brief Writes JSON to a file without ever leaving a half-written one.
     *
     * Three things the naive `ofstream out(path, trunc); out << j;` got
     * wrong, each of which has bitten this project:
     *
     *  1. It fails if the parent directory does not exist yet, so
     *     writing a generated registry into data/bin/ aborted on a
     *     fresh checkout. Directories are created here instead.
     *  2. `trunc` empties the target *before* the write, so a failure
     *     mid-way destroys the file that was there - and for the
     *     *.asset.json files that file is hand-authored source. We
     *     write to a sibling temp file and rename it into place, which
     *     is atomic on every platform we target.
     *  3. Nothing checked the stream *after* writing, so a full disk
     *     silently produced a truncated file. The stream state is
     *     verified before the rename.
     *
     * @throws std::runtime_error with the path in the message.
     */
    inline void write_json_file(const std::filesystem::path& path, const nlohmann::json& j, const int indent = 4) {
        const std::filesystem::path parent = path.parent_path();
        if (!parent.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parent, ec);
            if (ec) {
                throw std::runtime_error("Cannot create directory '" + parent.string() + "': " + ec.message());
            }
        }

        std::filesystem::path tmp = path;
        tmp += ".tmp";

        {
            std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
            if (!out) {
                throw std::runtime_error("Failed to open file for writing: " + tmp.string());
            }

            out << j.dump(indent);
            out.flush();

            if (!out) {
                throw std::runtime_error("Failed to write file: " + tmp.string());
            }
        }

        std::error_code ec;
        std::filesystem::rename(tmp, path, ec);
        if (ec) {
            std::filesystem::remove(tmp, ec);
            throw std::runtime_error("Failed to replace '" + path.string() + "': " + ec.message());
        }
    }

} // namespace sif::io

#endif // RENDER_ENGINE_IO_FROM_JSON_H