/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-17
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_RB_FROM_JSON_H
#define RENDER_ENGINE_RB_FROM_JSON_H

#include "sif/io/from_JSON.h"

#include "sif/asset/internal/AssetDesc.h"
#include "sif/asset/internal/data/AssetMetaData.h"
#include "sif/asset/internal/data/FontNode.h"
#include "sif/asset/internal/data/PrimitiveAnimationNode.h"
#include "sif/asset/internal/data/SoundNode.h"
#include "sif/asset/internal/data/SpriteAtlasNode.h"
#include "sif/asset/internal/data/SpriteGridNode.h"
#include "sif/asset/internal/data/SpriteSingleNode.h"
#include "sif/internal/Rect.h"

namespace sif::intrnl {
    inline void from_json(const nlohmann::json& j, Rect& r) {
        r.x = io::get_checked<float>(j, "x");
        r.y = io::get_checked<float>(j, "y");
        r.width = io::get_checked<float>(j, "width");
        r.height = io::get_checked<float>(j, "height");
    }

    inline void to_json(nlohmann::json& j, const Rect& r) {
        j = nlohmann::json{{"x", r.x}, {"y", r.y}, {"width", r.width}, {"height", r.height}};
    }
} // namespace sif::intrnl

namespace sif::asset::data {
    /**
     * @brief Reads AssetMetaData::record_id_to_name.
     *
     * JSON object keys are always strings, so the natural spelling of
     * an id -> name table is {"0": "heart", "1": "gem"}. nlohmann
     * cannot deserialize that into a map with an integer key on its
     * own (it insists on an array of pairs), which is why this is done
     * by hand. The legacy array-of-pairs form is still accepted so
     * registries written by an earlier build keep loading.
     */
    inline std::unordered_map<uint32_t, std::string> read_record_names(const nlohmann::json& j) {
        std::unordered_map<uint32_t, std::string> out;
        if (!j.contains("record_id_to_name")) {
            return out;
        }

        const nlohmann::json& node = j.at("record_id_to_name");

        if (node.is_object()) {
            for (const auto& [key, value] : node.items()) {
                out.emplace(static_cast<uint32_t>(std::stoul(key)), value.get<std::string>());
            }
            return out;
        }

        if (node.is_array()) {
            for (const auto& entry : node) {
                if (entry.is_array() && entry.size() == 2) {
                    out.emplace(entry[0].get<uint32_t>(), entry[1].get<std::string>());
                }
            }
        }
        return out;
    }

    /**
     * @brief Writes the id -> name table back as a JSON object.
     */
    inline nlohmann::json write_record_names(const std::unordered_map<uint32_t, std::string>& names) {
        nlohmann::json out = nlohmann::json::object();
        for (const auto& [id, name] : names) {
            out[std::to_string(id)] = name;
        }
        return out;
    }

    inline void from_json(const nlohmann::json& j, AssetMetaData& d) {
        d.guid = intrnl::GUID(io::get_checked<std::string>(j, "guid"));
        d.type = from_string(io::get_checked<std::string>(j, "type"));
        d.asset_name = io::get_checked<std::string>(j, "asset_name");

        d.expected_load_time_seconds =
            io::get_checked<double>(j, "expected_load_time_seconds", d.expected_load_time_seconds);

        d.critical = io::get_checked<bool>(j, "critical", d.critical);

        d.record_id_to_name = read_record_names(j);
        d.record_name_to_id.reserve(d.record_id_to_name.size());
        for (const auto& [id, name] : d.record_id_to_name) {
            d.record_name_to_id.emplace(name, id);
        }
    }

    inline void to_json(nlohmann::json& j, const AssetMetaData& d) {
        j = nlohmann::json{
            {"guid", d.guid.string()},    {"type", to_string(d.type)},
            {"asset_name", d.asset_name}, {"expected_load_time_seconds", d.expected_load_time_seconds},
            {"critical", d.critical},     {"record_id_to_name", write_record_names(d.record_id_to_name)}};
    }

    inline void from_json(const nlohmann::json& j, FontNode& d) {
        d.meta = io::get_checked<data::AssetMetaData>(j);
        d.source = io::get_checked<std::string>(j, "source");
    }

    inline void from_json(const nlohmann::json& j, SpriteSingleNode& d) {
        d.meta = io::get_checked<data::AssetMetaData>(j);
        d.source = io::get_checked<std::string>(j, "source");
    }

    inline void from_json(const nlohmann::json& j, SpriteAtlasNode& d) {
        d.meta = io::get_checked<data::AssetMetaData>(j);
        d.source = io::get_checked<std::string>(j, "source");
        // Optional: an atlas with no "rects" yet is valid (0 sub-sprites),
        // so an existing asset file without this field does not break.
        d.rects = io::get_checked<std::vector<intrnl::Rect>>(j, "rects", d.rects);
    }

    inline void to_json(nlohmann::json& j, const SpriteAtlasNode& d) {
        j = nlohmann::json{{"meta", d.meta}, {"source", d.source}, {"rects", d.rects}};
    }

    inline void from_json(const nlohmann::json& j, SpriteGridNode& d) {
        d.meta = io::get_checked<data::AssetMetaData>(j);
        d.source = io::get_checked<std::string>(j, "source");
        d.rows = io::get_checked<uint32_t>(j, "rows", d.rows);
        d.cols = io::get_checked<uint32_t>(j, "cols", d.cols);
        d.cell_width = io::get_checked<float>(j, "cell_width", d.cell_width);
        d.cell_height = io::get_checked<float>(j, "cell_height", d.cell_height);
    }

    inline void to_json(nlohmann::json& j, const SpriteGridNode& d) {
        j = nlohmann::json{{"meta", d.meta}, {"source", d.source},         {"rows", d.rows},
                           {"cols", d.cols}, {"cell_width", d.cell_width}, {"cell_height", d.cell_height}};
    }

    inline void from_json(const nlohmann::json& j, PrimitiveAnimationNode& d) {
        d.meta = io::get_checked<data::AssetMetaData>(j);
        d.source = io::get_checked<std::string>(j, "source");
        d.frames = io::get_checked<std::vector<intrnl::Rect>>(j, "frames", d.frames);
        d.frame_duration_seconds = io::get_checked<float>(j, "frame_duration_seconds", d.frame_duration_seconds);
        d.loop = io::get_checked<bool>(j, "loop", d.loop);
    }

    inline void to_json(nlohmann::json& j, const PrimitiveAnimationNode& d) {
        j = nlohmann::json{{"meta", d.meta},
                           {"source", d.source},
                           {"frames", d.frames},
                           {"frame_duration_seconds", d.frame_duration_seconds},
                           {"loop", d.loop}};
    }

    inline void from_json(const nlohmann::json& j, SoundNode& d) {
        d.meta = io::get_checked<data::AssetMetaData>(j);
        d.source = io::get_checked<std::string>(j, "source");
    }
} // namespace sif::asset::data

namespace sif::asset {
    inline void from_json(const nlohmann::json& j, AssetDesc& d) {
        d.meta = io::get_checked<data::AssetMetaData>(j, "meta");
        d.conf_path = io::get_checked<std::string>(j, "conf_path");
    }
    inline void to_json(nlohmann::json& j, const AssetDesc& d) {
        j = nlohmann::json{{"meta", d.meta}, {"conf_path", d.conf_path}};
    }

} // namespace sif::asset

#endif // RENDER_ENGINE_RB_FROM_JSON_H
