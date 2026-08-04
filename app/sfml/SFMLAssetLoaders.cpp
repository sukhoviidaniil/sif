/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "SFMLAssetLoaders.h"

#include <memory>
#include <stdexcept>

#include "SFMLAssets.h"

#include "sif/asset/internal/data/AssetDataLoader.h"
#include "sif/diagnostics/Logger.h"

namespace sif::sfml {
    namespace {
        /**
         * @brief Loads a texture or throws a message naming the file.
         *
         * SFML reports failure through a bool return; turning that into
         * an exception here is what lets IAssetLoader::try_load mark the
         * record Failed instead of silently handing out a blank texture.
         */
        sf::Texture load_texture(const std::string& path) {
            sf::Texture texture;
            if (!texture.loadFromFile(path)) {
                const std::string err = "SFML: unable to load texture '" + path + "'";
                LOG(err);
                throw std::runtime_error(err);
            }
            return texture;
        }

        sf::SoundBuffer load_sound_buffer(const std::string& path) {
            sf::SoundBuffer buffer;
            if (!buffer.loadFromFile(path)) {
                const std::string err =
                    "SFML: unable to load sound '" + path +
                    "' (SFML 2.6 decodes wav/ogg/flac, but not mp3)";
                LOG(err);
                throw std::runtime_error(err);
            }
            return buffer;
        }

        /**
         * @brief Erases a backend object to shared_ptr<void> *through*
         * its engine-facing base.
         *
         * Doing the upcast before the erase is what makes the reverse
         * cast in ui::Sprite / SFML_Renderer well defined.
         */
        template<typename Base, typename Concrete, typename... Args>
        std::shared_ptr<void> make_erased(Args&&... args) {
            const std::shared_ptr<Base> base = std::make_shared<Concrete>(std::forward<Args>(args)...);
            return std::static_pointer_cast<void>(base);
        }
    }

    void SFML_Font_AssetLoader::load_asset(asset::AssetRecord &record, const std::string &asset_dir, const uint64_t attempt_token) {
        const auto node = asset::data::AssetDataLoader::load_Font_from_file(asset_dir + record.get_conf_path());

        sf::Font font;
        const std::string path = asset_dir + node->source;
        if (!font.loadFromFile(path)) {
            const std::string err = "SFML: unable to load font '" + path + "'";
            LOG(err);
            throw std::runtime_error(err);
        }

        record.set_data_if_current(attempt_token, make_erased<asset::Font, SFMLFontAsset>(std::move(font)));
    }

    void SFML_SpriteSingle_AssetLoader::load_asset(asset::AssetRecord &record, const std::string &asset_dir, const uint64_t attempt_token) {
        const auto node = asset::data::AssetDataLoader::load_SpriteSingle_from_file(asset_dir + record.get_conf_path());

        record.set_data_if_current(
            attempt_token,
            make_erased<asset::Sprite, SFMLSpriteAsset>(load_texture(asset_dir + node->source))
        );
    }

    void SFML_SpriteAtlas_AssetLoader::load_asset(asset::AssetRecord &record, const std::string &asset_dir, const uint64_t attempt_token) {
        const auto node = asset::data::AssetDataLoader::load_SpriteAtlas_from_file(asset_dir + record.get_conf_path());

        math::Vector<intrnl::Rect> rects(node->rects.size());
        for (size_t i = 0; i < node->rects.size(); ++i) {
            rects[i] = node->rects[i];
        }

        record.set_data_if_current(
            attempt_token,
            make_erased<asset::SpriteAtlas, SFMLSpriteAtlasAsset>(
                std::move(rects), load_texture(asset_dir + node->source))
        );
    }

    void SFML_SpriteGrid_AssetLoader::load_asset(asset::AssetRecord &record, const std::string &asset_dir, const uint64_t attempt_token) {
        const auto node = asset::data::AssetDataLoader::load_SpriteGrid_from_file(asset_dir + record.get_conf_path());

        // The JSON only stores rows/cols/cell size; tiling it into
        // explicit per-cell rects happens once here, so nothing
        // downstream has to redo the arithmetic per frame.
        math::Matrix<intrnl::Rect> cells(node->rows, node->cols);
        for (uint32_t row = 0; row < node->rows; ++row) {
            for (uint32_t col = 0; col < node->cols; ++col) {
                cells(row, col) = intrnl::Rect(
                    static_cast<float>(col) * node->cell_width,
                    static_cast<float>(row) * node->cell_height,
                    node->cell_width,
                    node->cell_height
                );
            }
        }

        record.set_data_if_current(
            attempt_token,
            make_erased<asset::SpriteGrid, SFMLSpriteGridAsset>(
                std::move(cells), load_texture(asset_dir + node->source))
        );
    }

    void SFML_PrimitiveAnimation_AssetLoader::load_asset(asset::AssetRecord &record, const std::string &asset_dir, const uint64_t attempt_token) {
        const auto node = asset::data::AssetDataLoader::load_PrimitiveAnimation_from_file(asset_dir + record.get_conf_path());

        math::Vector<intrnl::Rect> frames(node->frames.size());
        for (size_t i = 0; i < node->frames.size(); ++i) {
            frames[i] = node->frames[i];
        }

        record.set_data_if_current(
            attempt_token,
            make_erased<asset::PrimitiveAnimation, SFMLPrimitiveAnimationAsset>(
                std::move(frames),
                node->frame_duration_seconds,
                node->loop,
                load_texture(asset_dir + node->source))
        );
    }

    void SFML_Sound_AssetLoader::load_asset(asset::AssetRecord &record, const std::string &asset_dir, const uint64_t attempt_token) {
        const auto node = asset::data::AssetDataLoader::load_Sound_from_file(asset_dir + record.get_conf_path());

        record.set_data_if_current(
            attempt_token,
            make_erased<asset::Sound, SFMLSoundAsset>(load_sound_buffer(asset_dir + node->source))
        );
    }

    void register_sfml_asset_loaders(asset::AssetRegistry &registry) {
        registry.register_loader(asset::AssetType::Font, std::make_unique<SFML_Font_AssetLoader>());
        registry.register_loader(asset::AssetType::SpriteSingle, std::make_unique<SFML_SpriteSingle_AssetLoader>());
        registry.register_loader(asset::AssetType::SpriteAtlas, std::make_unique<SFML_SpriteAtlas_AssetLoader>());
        registry.register_loader(asset::AssetType::SpriteGrid, std::make_unique<SFML_SpriteGrid_AssetLoader>());
        registry.register_loader(asset::AssetType::PrimitiveAnimation, std::make_unique<SFML_PrimitiveAnimation_AssetLoader>());
        registry.register_loader(asset::AssetType::Sound, std::make_unique<SFML_Sound_AssetLoader>());
    }
}
