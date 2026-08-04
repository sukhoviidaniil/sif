/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETS_SPRITE_H
#define RENDER_ENGINE_ASSETS_SPRITE_H

#include "sif/math/Vector2.h"

namespace sif::asset {
    /**
     * @brief A single, whole-texture sprite (AssetType::SpriteSingle).
     *
     * Backend-agnostic interface; the actual pixel data lives in a
     * backend-specific subclass constructed by that backend's asset
     * loader (e.g. sif::sfml::SFMLSprite, built by
     * SFML_Sprite_AssetLoader).
     */
    class Sprite {
    public:
        Sprite(const Sprite&) = delete;
        Sprite& operator=(const Sprite&) = delete;

        Sprite();
        virtual ~Sprite();
        Sprite(Sprite&&) noexcept;
        Sprite& operator=(Sprite&&) noexcept;

        /**
         * @brief Returns the pixel size of the sprite's texture.
         */
        [[nodiscard]] virtual math::Vector2 size() const = 0;
    };
}

#endif //RENDER_ENGINE_ASSETS_SPRITE_H
