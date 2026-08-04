/***************************************************************
 * Project:       Render_Engine (headless backend)
 * File:          HeadlessBackend.h
 *
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_HEADLESS_BACKEND_H
#define RENDER_ENGINE_HEADLESS_BACKEND_H

#include <string>

#include "SFML/Audio/InputSoundFile.hpp"
#include "SFML/Graphics/Image.hpp"

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/Font.h"
#include "sif/asset/internal/IAssetLoader.h"
#include "sif/asset/internal/PrimitiveAnimation.h"
#include "sif/asset/internal/Sound.h"
#include "sif/asset/internal/Sprite.h"
#include "sif/asset/internal/SpriteAtlas.h"
#include "sif/asset/internal/SpriteGrid.h"

/**
 * @file
 *
 * A second, deliberately minimal backend: it decodes assets on the CPU
 * only - sf::Image instead of sf::Texture, sf::SoundBuffer without ever
 * opening an output device - so it needs neither a GPU, an X11 display
 * nor a sound card.
 *
 * Two reasons it exists rather than being a test double:
 *
 *  1. It is the proof that the abstract factory is real. Everything
 *     above asset::* is written against the engine interfaces, so a
 *     whole new backend is a handful of classes plus one branch in
 *     Graphics_Factory - no change anywhere in sif/.
 *  2. It makes the asset pipeline verifiable in CI. The SFML *render*
 *     backend cannot even load a texture on a headless build agent
 *     (sf::Texture needs an OpenGL context), so without this the
 *     "sprites, animations and sounds really load" claim could only be
 *     checked by hand on a desktop.
 */
namespace sif::headless {

    class HeadlessFontAsset final : public asset::Font {
    public:
        HeadlessFontAsset() = default;

        /**
         * @brief Approximates text extents without rasterising glyphs.
         *
         * Real metrics would require uploading a glyph atlas, which is
         * exactly what this backend avoids; the estimate is documented
         * as such so nobody mistakes it for layout-accurate.
         */
        [[nodiscard]] math::Vector2 measure_text(const std::string& str, unsigned int size) const override {
            return {
                static_cast<float>(str.size()) * static_cast<float>(size) * 0.55f,
                static_cast<float>(size) * 1.2f
            };
        }
    };

    class HeadlessSpriteAsset final : public asset::Sprite {
    public:
        explicit HeadlessSpriteAsset(const sf::Image& image)
            : size_(static_cast<float>(image.getSize().x), static_cast<float>(image.getSize().y)) {}

        [[nodiscard]] math::Vector2 size() const override { return size_; }

    private:
        math::Vector2 size_;
    };

    class HeadlessSpriteAtlasAsset final : public asset::SpriteAtlas {
    public:
        HeadlessSpriteAtlasAsset(math::Vector<intrnl::Rect> rects, const sf::Image& image)
            : asset::SpriteAtlas(std::move(rects))
            , size_(static_cast<float>(image.getSize().x), static_cast<float>(image.getSize().y)) {}

        [[nodiscard]] math::Vector2 texture_size() const override { return size_; }

    private:
        math::Vector2 size_;
    };

    class HeadlessSpriteGridAsset final : public asset::SpriteGrid {
    public:
        HeadlessSpriteGridAsset(math::Matrix<intrnl::Rect> cells, const sf::Image& image)
            : asset::SpriteGrid(std::move(cells))
            , size_(static_cast<float>(image.getSize().x), static_cast<float>(image.getSize().y)) {}

        [[nodiscard]] math::Vector2 texture_size() const override { return size_; }

    private:
        math::Vector2 size_;
    };

    class HeadlessPrimitiveAnimationAsset final : public asset::PrimitiveAnimation {
    public:
        HeadlessPrimitiveAnimationAsset(math::Vector<intrnl::Rect> frames,
                                        const float frame_duration_seconds,
                                        const bool loop,
                                        const sf::Image& image)
            : asset::PrimitiveAnimation(std::move(frames), frame_duration_seconds, loop)
            , size_(static_cast<float>(image.getSize().x), static_cast<float>(image.getSize().y)) {}

        [[nodiscard]] math::Vector2 texture_size() const override { return size_; }

    private:
        math::Vector2 size_;
    };

    class HeadlessSoundAsset final : public asset::Sound {
    public:
        HeadlessSoundAsset(const float duration_seconds,
                           const unsigned int sample_rate,
                           const unsigned int channel_count)
            : duration_seconds_(duration_seconds)
            , sample_rate_(sample_rate)
            , channel_count_(channel_count) {}

        [[nodiscard]] float duration_seconds() const override { return duration_seconds_; }

        [[nodiscard]] unsigned int sample_rate() const { return sample_rate_; }
        [[nodiscard]] unsigned int channel_count() const { return channel_count_; }

    private:
        float duration_seconds_;
        unsigned int sample_rate_;
        unsigned int channel_count_;
    };

    /**
     * @brief Installs CPU-only loaders for every AssetType.
     */
    void register_headless_asset_loaders(asset::AssetRegistry& registry);
}

#endif //RENDER_ENGINE_HEADLESS_BACKEND_H
