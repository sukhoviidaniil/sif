/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/Sprite.h"

#include "sif/asset/internal/SpriteAtlas.h"
#include "sif/asset/internal/SpriteGrid.h"
#include "sif/render/elements/Sprite.h"

#include <algorithm>

namespace sif::ui {
    Sprite::Sprite(asset::AssetHandle<void> handle, const asset::AssetType kind, const intrnl::RecordID record_id)
        : asset_(std::move(handle)), kind_(kind), record_id_(record_id) {}

    math::Vector2 Sprite::measure(const math::Vector2& available) {
        // Auto keeps the old behaviour (fill whatever the parent gives
        // us); PX/PERCENT are honoured through the shared resolver, so
        // a width/height set from XML is no longer silently ignored.
        math::Vector2 size = resolve_size(available);

        size.x = std::max(0.f, size.x);
        size.y = std::max(0.f, size.y);

        return size;
    }

    void Sprite::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& /*ctx*/) const {
        if (!visible || !asset_.ready()) {
            return;
        }

        auto item = std::make_unique<rnd::Sprite>();
        item->rect = result.rect;
        item->asset = asset_;
        item->kind = kind_;
        item->src_rect = resolve_src_rect();
        item->tint = tint;

        frame.temp_items.push_back(std::move(item));
    }

    void Sprite::set_record_id(const intrnl::RecordID record_id) {
        record_id_ = record_id;
    }

    intrnl::RecordID Sprite::record_id() const {
        return record_id_;
    }

    bool Sprite::ready() const {
        return asset_.ready();
    }

    intrnl::Rect Sprite::resolve_src_rect() const {
        switch (kind_) {
        case asset::AssetType::SpriteAtlas: {
            const auto* atlas = static_cast<const asset::SpriteAtlas*>(asset_.get());
            if (atlas == nullptr || record_id_.id >= atlas->count()) {
                return {};
            }
            return atlas->rect(record_id_);
        }
        case asset::AssetType::SpriteGrid: {
            const auto* grid = static_cast<const asset::SpriteGrid*>(asset_.get());
            if (grid == nullptr || grid->cols() == 0) {
                return {};
            }
            if (record_id_.id >= grid->rows() * grid->cols()) {
                return {};
            }
            return grid->cell(record_id_);
        }
        default:
            // SpriteSingle (and anything unexpected): whole texture.
            return {};
        }
    }
} // namespace sif::ui
