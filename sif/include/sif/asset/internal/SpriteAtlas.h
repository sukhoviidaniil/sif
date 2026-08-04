/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETS_SPRITEATLAS_H
#define RENDER_ENGINE_ASSETS_SPRITEATLAS_H

#include <cstddef>

#include "sif/math/Vector.h"
#include "sif/math/Vector2.h"
#include "sif/internal/Rect.h"
#include "sif/internal/RecordID.h"

namespace sif::asset {
    /**
     * @brief One shared texture cut into a named, indexable list of
     * sub-sprites (AssetType::SpriteAtlas) - "a vector of sprites".
     *
     * The sub-rect geometry is backend-agnostic and stored directly as
     * a math::Vector<intrnl::Rect>, one entry per record id (see
     * intrnl::RecordID and AssetMetaData::record_id_to_name for
     * optional human-readable names). The actual pixel data lives in a
     * backend-specific subclass that only needs to implement
     * texture_size() (e.g. sif::sfml::SFMLSpriteAtlas, built by
     * SFML_SpriteAtlas_AssetLoader).
     */
    class SpriteAtlas {
    public:
        SpriteAtlas(const SpriteAtlas&) = delete;
        SpriteAtlas& operator=(const SpriteAtlas&) = delete;

        explicit SpriteAtlas(math::Vector<intrnl::Rect> rects);
        virtual ~SpriteAtlas();
        SpriteAtlas(SpriteAtlas&&) noexcept;
        SpriteAtlas& operator=(SpriteAtlas&&) noexcept;

        /**
         * @brief Number of sub-sprites in this atlas.
         */
        [[nodiscard]] size_t count() const;

        /**
         * @brief Returns the pixel rect of the sub-sprite at id.
         *
         * Throws std::out_of_range if id is outside [0, count()).
         */
        [[nodiscard]] const intrnl::Rect& rect(intrnl::RecordID id) const;

        /**
         * @brief Returns the pixel size of the whole shared texture.
         */
        [[nodiscard]] virtual math::Vector2 texture_size() const = 0;

    private:
        math::Vector<intrnl::Rect> rects_;
    };
}

#endif //RENDER_ENGINE_ASSETS_SPRITEATLAS_H
