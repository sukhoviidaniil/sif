/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/SpriteAtlas.h"

#include <stdexcept>

namespace sif::asset {
    SpriteAtlas::SpriteAtlas(math::Vector<intrnl::Rect> rects)
        : rects_(std::move(rects)) {
    }

    SpriteAtlas::~SpriteAtlas() = default;

    SpriteAtlas::SpriteAtlas(SpriteAtlas&&) noexcept = default;

    SpriteAtlas& SpriteAtlas::operator=(SpriteAtlas&&) noexcept = default;

    size_t SpriteAtlas::count() const {
        return rects_.size();
    }

    const intrnl::Rect& SpriteAtlas::rect(intrnl::RecordID id) const {
        if (id.id >= rects_.size()) {
            throw std::out_of_range("SpriteAtlas::rect - id out of range: " + id.string());
        }
        return rects_[id.id];
    }
}
