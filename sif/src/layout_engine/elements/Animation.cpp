/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-08-03
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/layout_engine/elements/Animation.h"

#include "sif/render/elements/Sprite.h"

#include <algorithm>

namespace sif::ui {
    Animation::Animation(asset::AssetHandle<asset::PrimitiveAnimation> handle)
        : asset_(std::move(handle)) {
    }

    math::Vector2 Animation::measure(const math::Vector2 &available) {
        math::Vector2 size = resolve_size(available);
        size.x = std::max(0.f, size.x);
        size.y = std::max(0.f, size.y);
        return size;
    }

    void Animation::append_render_items(rnd::RenderFrame &frame, const rnd::FrameContext & /*ctx*/) const {
        if (!visible || !asset_.ready()) {
            return;
        }

        const asset::PrimitiveAnimation* animation = asset_.get();
        if (animation == nullptr || animation->frame_count() == 0) {
            return;
        }

        auto item = std::make_unique<rnd::Sprite>();
        item->rect = result.rect;
        item->asset = asset::AssetHandle<void>(asset_.record());
        item->kind = asset::AssetType::PrimitiveAnimation;
        item->src_rect = animation->frame_at(elapsed_seconds_);
        item->tint = tint;

        // Animated frames change every tick, so they belong in the
        // per-frame bucket rather than the "constant" one.
        frame.temp_items.push_back(std::move(item));
    }

    void Animation::update(const float dt) {
        if (playing_) {
            elapsed_seconds_ += dt * speed;
        }
        UIElement::update(dt);
    }

    void Animation::play() {
        playing_ = true;
    }

    void Animation::pause() {
        playing_ = false;
    }

    void Animation::restart() {
        elapsed_seconds_ = 0.f;
    }

    bool Animation::playing() const {
        return playing_;
    }

    float Animation::elapsed_seconds() const {
        return elapsed_seconds_;
    }

    size_t Animation::current_frame() const {
        const asset::PrimitiveAnimation* animation = asset_.get();
        if (animation == nullptr) {
            return 0;
        }
        return animation->frame_index_at(elapsed_seconds_);
    }
}
