/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETS_PRIMITIVEANIMATION_H
#define RENDER_ENGINE_ASSETS_PRIMITIVEANIMATION_H

#include <cstddef>

#include "sif/math/Vector.h"
#include "sif/math/Vector2.h"
#include "sif/internal/Rect.h"

namespace sif::asset {
    /**
     * @brief A shared texture animated as a series of frames, advanced
     * by a transition timer (AssetType::PrimitiveAnimation).
     *
     * Frame geometry is backend-agnostic and stored directly as a
     * math::Vector<intrnl::Rect>, one rect per frame, all sharing one
     * texture (see data::PrimitiveAnimationNode). The actual pixel data
     * lives in a backend-specific subclass that only needs to
     * implement texture_size() (e.g. sif::sfml::SFMLPrimitiveAnimation,
     * built by SFML_PrimitiveAnimation_AssetLoader).
     *
     * The timer logic itself (which frame is showing after t seconds
     * of playback) is centralized in frame_index_at()/frame_at(), so
     * every consumer (UI Animation element, gameplay code, etc.) gets
     * identical, tested behavior instead of reimplementing it.
     */
    class PrimitiveAnimation {
    public:
        PrimitiveAnimation(const PrimitiveAnimation&) = delete;
        PrimitiveAnimation& operator=(const PrimitiveAnimation&) = delete;

        /**
         * @brief Constructs an animation from its frame rects and timing.
         *
         * @param frames Per-frame pixel rects, all within the same texture.
         * @param frame_duration_seconds How long each frame is shown, in seconds.
         * @param loop Whether playback restarts at frame 0 after the last frame.
         */
        PrimitiveAnimation(math::Vector<intrnl::Rect> frames, float frame_duration_seconds, bool loop);
        virtual ~PrimitiveAnimation();
        PrimitiveAnimation(PrimitiveAnimation&&) noexcept;
        PrimitiveAnimation& operator=(PrimitiveAnimation&&) noexcept;

        /**
         * @brief Number of frames in the animation.
         */
        [[nodiscard]] size_t frame_count() const;

        /**
         * @brief How long each frame is shown, in seconds.
         */
        [[nodiscard]] float frame_duration_seconds() const;

        /**
         * @brief Whether playback loops after the last frame.
         */
        [[nodiscard]] bool loops() const;

        /**
         * @brief Total duration of one full, non-looping playthrough, in seconds.
         */
        [[nodiscard]] float total_duration_seconds() const;

        /**
         * @brief Resolves which frame index is showing after elapsed_seconds
         * of continuous playback starting at frame 0.
         *
         * If loop() is true, elapsed_seconds wraps around
         * (fmod-style); otherwise it clamps to the last frame once
         * elapsed_seconds exceeds total_duration_seconds(). Returns 0
         * for an animation with no frames (frame_count() == 0).
         *
         * @param elapsed_seconds Time since animation start, in seconds.
         */
        [[nodiscard]] size_t frame_index_at(float elapsed_seconds) const;

        /**
         * @brief Returns the pixel rect of the frame showing at elapsed_seconds.
         *
         * Equivalent to rect_of(frame_index_at(elapsed_seconds)).
         */
        [[nodiscard]] const intrnl::Rect& frame_at(float elapsed_seconds) const;

        /**
         * @brief Returns the pixel rect of a specific frame index.
         *
         * Throws std::out_of_range if index is outside [0, frame_count()).
         */
        [[nodiscard]] const intrnl::Rect& rect_of(size_t index) const;

        /**
         * @brief Returns the pixel size of the whole shared texture.
         */
        [[nodiscard]] virtual math::Vector2 texture_size() const = 0;

    private:
        math::Vector<intrnl::Rect> frames_;
        float frame_duration_seconds_;
        bool loop_;
    };
}

#endif //RENDER_ENGINE_ASSETS_PRIMITIVEANIMATION_H
