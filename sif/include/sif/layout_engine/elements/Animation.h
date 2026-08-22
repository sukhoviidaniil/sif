/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-08-03
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UI_ANIMATION_H
#define RENDER_ENGINE_UI_ANIMATION_H

#include "UIElement.h"

#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/PrimitiveAnimation.h"
#include "sif/internal/Color.h"

namespace sif::ui {
    /**
     * @brief Plays an asset::PrimitiveAnimation inside the layout tree.
     *
     * The element owns only the playback *cursor* (elapsed seconds,
     * playing/paused, speed). Which frame that cursor maps to is
     * answered by the asset itself (PrimitiveAnimation::frame_index_at),
     * so the timing rules - wrap vs. hold on the last frame - exist in
     * exactly one, unit-tested place instead of being reimplemented per
     * element. The frame's pixel rect is then handed to the renderer as
     * a plain rnd::Sprite, which is why no animation-specific code is
     * needed in any backend.
     */
    class Animation : public UIElement {
    public:
        explicit Animation(asset::AssetHandle<asset::PrimitiveAnimation> handle);

        math::Vector2 measure(const math::Vector2& available) override;

        void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const override;

        /**
         * @brief Advances the playback cursor by dt seconds (scaled by speed).
         *
         * Does nothing while paused, so pausing is a genuine freeze
         * rather than "keep counting and hope nobody looks".
         */
        void update(float dt) override;

        void play();
        void pause();
        void restart();

        [[nodiscard]] bool playing() const;
        [[nodiscard]] float elapsed_seconds() const;

        /**
         * @brief Index of the frame currently on screen.
         *
         * Returns 0 while the asset is still loading, so callers can
         * use this before the animation exists without special-casing.
         */
        [[nodiscard]] size_t current_frame() const;

        /// @brief Playback rate multiplier (1 = as authored, 2 = twice as fast).
        float speed = 1.f;

        /// @brief Multiplied with the texture's colors when drawn.
        intrnl::Color tint{};

    private:
        asset::AssetHandle<asset::PrimitiveAnimation> asset_;
        float elapsed_seconds_ = 0.f;
        bool playing_ = true;
    };
} // namespace sif::ui

#endif // RENDER_ENGINE_UI_ANIMATION_H
