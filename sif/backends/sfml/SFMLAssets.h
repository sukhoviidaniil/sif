/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * Description:   Concrete, SFML-backed implementations of the
 *                backend-agnostic asset interfaces declared in
 *                the sif/asset/internal headers.
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SFML_SFMLASSETS_H
#define RENDER_ENGINE_SFML_SFMLASSETS_H

#include <string>
#include <utility>

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "sif/asset/internal/Font.h"
#include "sif/asset/internal/PrimitiveAnimation.h"
#include "sif/asset/internal/Sound.h"
#include "sif/asset/internal/Sprite.h"
#include "sif/asset/internal/SpriteAtlas.h"
#include "sif/asset/internal/SpriteGrid.h"

/**
 * @file
 *
 * One file for the whole family on purpose: these classes exist only
 * to bolt an sf::Texture / sf::Font / sf::SoundBuffer onto an engine
 * interface, they are each ~10 lines, and they change together
 * whenever the backend changes. Splitting them across six headers
 * would add six include cycles' worth of noise without adding a single
 * decision anyone can make independently.
 *
 * Ownership rule for every loader in this backend: build the object as
 * shared_ptr<ConcreteBackendType>, then upcast to the *engine* base
 * (asset::Sprite, asset::SpriteAtlas, ...) before erasing it to
 * shared_ptr<void> for AssetRecord. The pointer value stored in the
 * record is then already the engine-base pointer, so casting it back
 * out of void* is well defined - erasing the most-derived pointer
 * instead would only work by accident.
 */
namespace sif::sfml {

    /**
     * @brief Anything the renderer can draw from: exposes one texture.
     *
     * The renderer never dynamic_casts to this; it knows the concrete
     * type from rnd::Sprite::kind and static-casts. The interface
     * exists so the four texture-backed asset classes below cannot
     * accidentally disagree on how the texture is spelled.
     */
    class ITextureSource {
    public:
        virtual ~ITextureSource() = default;
        [[nodiscard]] virtual const sf::Texture& texture() const = 0;
    };

    // ---------------------------------------------------------------
    // Font
    // ---------------------------------------------------------------

    class SFMLFontAsset final : public asset::Font {
    public:
        explicit SFMLFontAsset(sf::Font font) : font_(std::move(font)) {}

        [[nodiscard]] const sf::Font& font() const { return font_; }

        /**
         * @brief Measures a string with the real font metrics.
         *
         * The layout engine only knows "a font can measure text"; this
         * is where that promise is actually kept, so ui::Text no longer
         * has to guess a width from character count.
         */
        [[nodiscard]] math::Vector2 measure_text(const std::string& str, unsigned int size) const override {
            sf::Text text;
            text.setFont(font_);
            text.setString(str);
            text.setCharacterSize(size);
            const sf::FloatRect bounds = text.getLocalBounds();
            return {bounds.width, bounds.height};
        }

    private:
        sf::Font font_;
    };

    // ---------------------------------------------------------------
    // Textures
    // ---------------------------------------------------------------

    class SFMLSpriteAsset final : public asset::Sprite, public ITextureSource {
    public:
        explicit SFMLSpriteAsset(sf::Texture texture) : texture_(std::move(texture)) {}

        [[nodiscard]] const sf::Texture& texture() const override { return texture_; }

        [[nodiscard]] math::Vector2 size() const override {
            return {static_cast<float>(texture_.getSize().x), static_cast<float>(texture_.getSize().y)};
        }

    private:
        sf::Texture texture_;
    };

    class SFMLSpriteAtlasAsset final : public asset::SpriteAtlas, public ITextureSource {
    public:
        SFMLSpriteAtlasAsset(math::Vector<intrnl::Rect> rects, sf::Texture texture)
            : asset::SpriteAtlas(std::move(rects)), texture_(std::move(texture)) {}

        [[nodiscard]] const sf::Texture& texture() const override { return texture_; }

        [[nodiscard]] math::Vector2 texture_size() const override {
            return {static_cast<float>(texture_.getSize().x), static_cast<float>(texture_.getSize().y)};
        }

    private:
        sf::Texture texture_;
    };

    class SFMLSpriteGridAsset final : public asset::SpriteGrid, public ITextureSource {
    public:
        SFMLSpriteGridAsset(math::Matrix<intrnl::Rect> cells, sf::Texture texture)
            : asset::SpriteGrid(std::move(cells)), texture_(std::move(texture)) {}

        [[nodiscard]] const sf::Texture& texture() const override { return texture_; }

        [[nodiscard]] math::Vector2 texture_size() const override {
            return {static_cast<float>(texture_.getSize().x), static_cast<float>(texture_.getSize().y)};
        }

    private:
        sf::Texture texture_;
    };

    class SFMLPrimitiveAnimationAsset final : public asset::PrimitiveAnimation, public ITextureSource {
    public:
        SFMLPrimitiveAnimationAsset(math::Vector<intrnl::Rect> frames,
                                    const float frame_duration_seconds,
                                    const bool loop,
                                    sf::Texture texture)
            : asset::PrimitiveAnimation(std::move(frames), frame_duration_seconds, loop)
            , texture_(std::move(texture)) {}

        [[nodiscard]] const sf::Texture& texture() const override { return texture_; }

        [[nodiscard]] math::Vector2 texture_size() const override {
            return {static_cast<float>(texture_.getSize().x), static_cast<float>(texture_.getSize().y)};
        }

    private:
        sf::Texture texture_;
    };

    // ---------------------------------------------------------------
    // Sound
    // ---------------------------------------------------------------

    class SFMLSoundAsset final : public asset::Sound {
    public:
        explicit SFMLSoundAsset(sf::SoundBuffer buffer) : buffer_(std::move(buffer)) {}

        [[nodiscard]] const sf::SoundBuffer& buffer() const { return buffer_; }

        [[nodiscard]] float duration_seconds() const override {
            return buffer_.getDuration().asSeconds();
        }

    private:
        sf::SoundBuffer buffer_;
    };
}

#endif //RENDER_ENGINE_SFML_SFMLASSETS_H
