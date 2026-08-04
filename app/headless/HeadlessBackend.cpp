/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "HeadlessBackend.h"

#include <memory>
#include <stdexcept>

#include "sif/asset/internal/data/AssetDataLoader.h"
#include <fstream>

#include "sif/diagnostics/Logger.h"

namespace sif::headless {
    namespace {
        sf::Image load_image(const std::string& path) {
            sf::Image image;
            if (!image.loadFromFile(path)) {
                const std::string err = "headless: unable to decode image '" + path + "'";
                LOG(err);
                throw std::runtime_error(err);
            }
            return image;
        }

        /**
         * @brief Reads a sound's format and length without an audio device.
         *
         * sf::SoundBuffer would work too, but it uploads the samples to
         * OpenAL and therefore needs a sound card; sf::InputSoundFile is
         * the pure decoder, which is all a headless check needs.
         */
        std::unique_ptr<HeadlessSoundAsset> probe_sound(const std::string& path) {
            sf::InputSoundFile file;
            if (!file.openFromFile(path)) {
                const std::string err =
                    "headless: unable to decode sound '" + path +
                    "' (wav/ogg/flac are supported, mp3 is not)";
                LOG(err);
                throw std::runtime_error(err);
            }
            return std::make_unique<HeadlessSoundAsset>(
                file.getDuration().asSeconds(),
                file.getSampleRate(),
                file.getChannelCount());
        }

        /// See SFMLAssets.h: erase to void *through* the engine base.
        template<typename Base, typename Concrete, typename... Args>
        std::shared_ptr<void> make_erased(Args&&... args) {
            const std::shared_ptr<Base> base = std::make_shared<Concrete>(std::forward<Args>(args)...);
            return std::static_pointer_cast<void>(base);
        }

        class FontLoader final : public asset::IAssetLoader {
            void load_asset(asset::AssetRecord& record, const std::string& asset_dir, const uint64_t token) override {
                // The file is still opened, so a missing/corrupt font
                // is reported here rather than at first draw.
                const auto node = asset::data::AssetDataLoader::load_Font_from_file(asset_dir + record.get_conf_path());
                const std::string path = asset_dir + node->source;
                if (std::ifstream probe(path, std::ios::binary); !probe) {
                    throw std::runtime_error("headless: unable to open font '" + path + "'");
                }
                record.set_data_if_current(token, make_erased<asset::Font, HeadlessFontAsset>());
            }
        };

        class SpriteSingleLoader final : public asset::IAssetLoader {
            void load_asset(asset::AssetRecord& record, const std::string& asset_dir, const uint64_t token) override {
                const auto node = asset::data::AssetDataLoader::load_SpriteSingle_from_file(asset_dir + record.get_conf_path());
                record.set_data_if_current(
                    token, make_erased<asset::Sprite, HeadlessSpriteAsset>(load_image(asset_dir + node->source)));
            }
        };

        class SpriteAtlasLoader final : public asset::IAssetLoader {
            void load_asset(asset::AssetRecord& record, const std::string& asset_dir, const uint64_t token) override {
                const auto node = asset::data::AssetDataLoader::load_SpriteAtlas_from_file(asset_dir + record.get_conf_path());

                math::Vector<intrnl::Rect> rects(node->rects.size());
                for (size_t i = 0; i < node->rects.size(); ++i) {
                    rects[i] = node->rects[i];
                }

                record.set_data_if_current(
                    token,
                    make_erased<asset::SpriteAtlas, HeadlessSpriteAtlasAsset>(
                        std::move(rects), load_image(asset_dir + node->source)));
            }
        };

        class SpriteGridLoader final : public asset::IAssetLoader {
            void load_asset(asset::AssetRecord& record, const std::string& asset_dir, const uint64_t token) override {
                const auto node = asset::data::AssetDataLoader::load_SpriteGrid_from_file(asset_dir + record.get_conf_path());

                math::Matrix<intrnl::Rect> cells(node->rows, node->cols);
                for (uint32_t row = 0; row < node->rows; ++row) {
                    for (uint32_t col = 0; col < node->cols; ++col) {
                        cells(row, col) = intrnl::Rect(
                            static_cast<float>(col) * node->cell_width,
                            static_cast<float>(row) * node->cell_height,
                            node->cell_width,
                            node->cell_height);
                    }
                }

                record.set_data_if_current(
                    token,
                    make_erased<asset::SpriteGrid, HeadlessSpriteGridAsset>(
                        std::move(cells), load_image(asset_dir + node->source)));
            }
        };

        class PrimitiveAnimationLoader final : public asset::IAssetLoader {
            void load_asset(asset::AssetRecord& record, const std::string& asset_dir, const uint64_t token) override {
                const auto node = asset::data::AssetDataLoader::load_PrimitiveAnimation_from_file(asset_dir + record.get_conf_path());

                math::Vector<intrnl::Rect> frames(node->frames.size());
                for (size_t i = 0; i < node->frames.size(); ++i) {
                    frames[i] = node->frames[i];
                }

                record.set_data_if_current(
                    token,
                    make_erased<asset::PrimitiveAnimation, HeadlessPrimitiveAnimationAsset>(
                        std::move(frames),
                        node->frame_duration_seconds,
                        node->loop,
                        load_image(asset_dir + node->source)));
            }
        };

        class SoundLoader final : public asset::IAssetLoader {
            void load_asset(asset::AssetRecord& record, const std::string& asset_dir, const uint64_t token) override {
                const auto node = asset::data::AssetDataLoader::load_Sound_from_file(asset_dir + record.get_conf_path());
                const std::shared_ptr<asset::Sound> sound = probe_sound(asset_dir + node->source);
                record.set_data_if_current(token, std::static_pointer_cast<void>(sound));
            }
        };
    }

    void register_headless_asset_loaders(asset::AssetRegistry &registry) {
        registry.register_loader(asset::AssetType::Font, std::make_unique<FontLoader>());
        registry.register_loader(asset::AssetType::SpriteSingle, std::make_unique<SpriteSingleLoader>());
        registry.register_loader(asset::AssetType::SpriteAtlas, std::make_unique<SpriteAtlasLoader>());
        registry.register_loader(asset::AssetType::SpriteGrid, std::make_unique<SpriteGridLoader>());
        registry.register_loader(asset::AssetType::PrimitiveAnimation, std::make_unique<PrimitiveAnimationLoader>());
        registry.register_loader(asset::AssetType::Sound, std::make_unique<SoundLoader>());
    }
}
