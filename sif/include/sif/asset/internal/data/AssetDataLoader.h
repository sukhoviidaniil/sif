/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-17
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETDATALOADER_H
#define RENDER_ENGINE_ASSETDATALOADER_H

#include <filesystem>

#include "AssetDataNode.h"
#include "FontNode.h"
#include "SpriteSingleNode.h"
#include "SpriteAtlasNode.h"
#include "SpriteGridNode.h"
#include "PrimitiveAnimationNode.h"
#include "SoundNode.h"


namespace sif::asset::data {
    class AssetDataLoader {
    public:

        static AssetDataLoader& instance();

        static std::unique_ptr<AssetDataNode> load_from_file(const std::filesystem::path& path);
        static void save_Meta(const std::filesystem::path& path, data::AssetMetaData& meta);

        static std::unique_ptr<FontNode> load_Font_from_file(const std::filesystem::path& path);
        static void save_Meta_Font(const std::filesystem::path& path, data::AssetMetaData& meta);

        static std::unique_ptr<SpriteSingleNode> load_SpriteSingle_from_file(const std::filesystem::path& path);
        static void save_Meta_SpriteSingle(const std::filesystem::path& path, data::AssetMetaData& meta);

        static std::unique_ptr<SpriteAtlasNode> load_SpriteAtlas_from_file(const std::filesystem::path& path);
        static void save_Meta_SpriteAtlas(const std::filesystem::path& path, data::AssetMetaData& meta);

        static std::unique_ptr<SpriteGridNode> load_SpriteGrid_from_file(const std::filesystem::path& path);
        static void save_Meta_SpriteGrid(const std::filesystem::path& path, data::AssetMetaData& meta);

        static std::unique_ptr<PrimitiveAnimationNode> load_PrimitiveAnimation_from_file(const std::filesystem::path& path);
        static void save_Meta_PrimitiveAnimation(const std::filesystem::path& path, data::AssetMetaData& meta);

        static std::unique_ptr<SoundNode> load_Sound_from_file(const std::filesystem::path& path);
        static void save_Meta_Sound(const std::filesystem::path& path, data::AssetMetaData& meta);

    private:
        AssetDataLoader();
    };
}

#endif //RENDER_ENGINE_ASSETDATALOADER_H