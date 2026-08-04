/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-17
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/data/AssetDataLoader.h"

#include "sif/internal/from_JSON.h"

namespace sif::asset::data {

    AssetDataLoader & AssetDataLoader::instance() {
        static AssetDataLoader instance;
        return instance;
    }

    std::unique_ptr<AssetDataNode> AssetDataLoader::load_from_file(const std::filesystem::path &path) {
        const nlohmann::json& j = io::get_json_data(path);
        const asset::AssetType type = from_string(io::get_checked<std::string>(j, "type"));
        switch (type) {
            case AssetType::Font:
                return load_Font_from_file(path);
            case AssetType::SpriteSingle:
                return load_SpriteSingle_from_file(path);
            case AssetType::SpriteAtlas:
                return load_SpriteAtlas_from_file(path);
            case AssetType::SpriteGrid:
                return load_SpriteGrid_from_file(path);
            case AssetType::PrimitiveAnimation:
                return load_PrimitiveAnimation_from_file(path);
            case AssetType::Sound:
                return load_Sound_from_file(path);
            default: return nullptr;
        }

    }

    void AssetDataLoader::save_Meta(const std::filesystem::path &path, data::AssetMetaData& meta) {
        switch (meta.type) {
            case AssetType::Font:
                save_Meta_Font(path, meta);
                break;
            case AssetType::SpriteSingle:
                save_Meta_SpriteSingle(path, meta);
                break;
            case AssetType::SpriteAtlas:
                save_Meta_SpriteAtlas(path, meta);
                break;
            case AssetType::SpriteGrid:
                save_Meta_SpriteGrid(path, meta);
                break;
            case AssetType::PrimitiveAnimation:
                save_Meta_PrimitiveAnimation(path, meta);
                break;
            case AssetType::Sound:
                save_Meta_Sound(path, meta);
                break;
            default: break;
        }
    }


    std::unique_ptr<FontNode> AssetDataLoader::load_Font_from_file(const std::filesystem::path &path) {
        const nlohmann::json& j = io::get_json_data(path);
        auto t = io::get_checked<FontNode>(j);

        return std::make_unique<FontNode>(t);
    }

    void AssetDataLoader::save_Meta_Font(const std::filesystem::path &path, data::AssetMetaData& meta) {
        nlohmann::json j = io::get_json_data(path);

        j["guid"] = meta.guid.string();
        j["asset_name"] = meta.asset_name;

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Failed to open file for writing: " + path.string());
        }

        out << j.dump(4);
    }

    std::unique_ptr<SpriteSingleNode> AssetDataLoader::load_SpriteSingle_from_file(const std::filesystem::path &path) {
        const nlohmann::json& j = io::get_json_data(path);
        auto t = io::get_checked<SpriteSingleNode>(j);

        return std::make_unique<SpriteSingleNode>(t);
    }

    void AssetDataLoader::save_Meta_SpriteSingle(const std::filesystem::path &path, data::AssetMetaData& meta) {
        nlohmann::json j = io::get_json_data(path);

        j["guid"] = meta.guid.string();
        j["asset_name"] = meta.asset_name;

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Failed to open file for writing: " + path.string());
        }

        out << j.dump(4);
    }

    std::unique_ptr<SpriteAtlasNode> AssetDataLoader::load_SpriteAtlas_from_file(const std::filesystem::path &path) {
        const nlohmann::json& j = io::get_json_data(path);
        auto t = io::get_checked<SpriteAtlasNode>(j);

        return std::make_unique<SpriteAtlasNode>(t);
    }

    void AssetDataLoader::save_Meta_SpriteAtlas(const std::filesystem::path &path, data::AssetMetaData& meta) {
        nlohmann::json j = io::get_json_data(path);

        j["guid"] = meta.guid.string();
        j["asset_name"] = meta.asset_name;
        j["record_id_to_name"] = meta.record_id_to_name;

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Failed to open file for writing: " + path.string());
        }

        out << j.dump(4);
    }

    std::unique_ptr<SpriteGridNode> AssetDataLoader::load_SpriteGrid_from_file(const std::filesystem::path &path) {
        const nlohmann::json& j = io::get_json_data(path);
        auto t = io::get_checked<SpriteGridNode>(j);

        return std::make_unique<SpriteGridNode>(t);
    }

    void AssetDataLoader::save_Meta_SpriteGrid(const std::filesystem::path &path, data::AssetMetaData& meta) {
        nlohmann::json j = io::get_json_data(path);

        j["guid"] = meta.guid.string();
        j["asset_name"] = meta.asset_name;
        j["record_id_to_name"] = meta.record_id_to_name;

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Failed to open file for writing: " + path.string());
        }

        out << j.dump(4);
    }

    std::unique_ptr<PrimitiveAnimationNode> AssetDataLoader::load_PrimitiveAnimation_from_file(const std::filesystem::path &path) {
        const nlohmann::json& j = io::get_json_data(path);
        auto t = io::get_checked<PrimitiveAnimationNode>(j);

        return std::make_unique<PrimitiveAnimationNode>(t);
    }

    void AssetDataLoader::save_Meta_PrimitiveAnimation(const std::filesystem::path &path, data::AssetMetaData& meta) {
        nlohmann::json j = io::get_json_data(path);

        j["guid"] = meta.guid.string();
        j["asset_name"] = meta.asset_name;
        j["record_id_to_name"] = meta.record_id_to_name;

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Failed to open file for writing: " + path.string());
        }

        out << j.dump(4);
    }

    std::unique_ptr<SoundNode> AssetDataLoader::load_Sound_from_file(const std::filesystem::path &path) {
        const nlohmann::json& j = io::get_json_data(path);
        auto t = io::get_checked<SoundNode>(j);

        return std::make_unique<SoundNode>(t);
    }

    void AssetDataLoader::save_Meta_Sound(const std::filesystem::path &path, data::AssetMetaData& meta) {
        nlohmann::json j = io::get_json_data(path);

        j["guid"] = meta.guid.string();
        j["asset_name"] = meta.asset_name;

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Failed to open file for writing: " + path.string());
        }

        out << j.dump(4);
    }

    AssetDataLoader::AssetDataLoader() = default;

}
