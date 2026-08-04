/***************************************************************
 * Project:       Render_Engine (SFML backend)
 * File:          SFMLAssetLoaders.h
 *
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SFML_SFMLASSETLOADERS_H
#define RENDER_ENGINE_SFML_SFMLASSETLOADERS_H

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/IAssetLoader.h"

namespace sif::sfml {
    /**
     * @brief IAssetLoader implementations, one per AssetType.
     *
     * Each of them does exactly three things: read the *.asset.json
     * node for the asset, build the matching SFML-backed object, and
     * hand it to the record with set_data_if_current(). They never
     * touch AssetState - that transition belongs to
     * IAssetLoader::try_load, which also enforces the promised load
     * time.
     */

    class SFML_Font_AssetLoader final : public asset::IAssetLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_SpriteSingle_AssetLoader final : public asset::IAssetLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_SpriteAtlas_AssetLoader final : public asset::IAssetLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_SpriteGrid_AssetLoader final : public asset::IAssetLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_PrimitiveAnimation_AssetLoader final : public asset::IAssetLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_Sound_AssetLoader final : public asset::IAssetLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    /**
     * @brief Registers one loader per AssetType with the registry.
     *
     * Called once during start-up; without it every request() would
     * fail with "There is no loader for: ...", which is exactly what
     * used to happen for everything except fonts.
     */
    void register_sfml_asset_loaders(asset::AssetRegistry& registry);
}

#endif //RENDER_ENGINE_SFML_SFMLASSETLOADERS_H
