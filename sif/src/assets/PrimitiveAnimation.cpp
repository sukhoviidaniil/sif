/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/PrimitiveAnimation.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace sif::asset {
    PrimitiveAnimation::PrimitiveAnimation(math::Vector<intrnl::Rect> frames, const float frame_duration_seconds, const bool loop)
        : frames_(std::move(frames))
        , frame_duration_seconds_(frame_duration_seconds)
        , loop_(loop) {
    }

    PrimitiveAnimation::~PrimitiveAnimation() = default;

    PrimitiveAnimation::PrimitiveAnimation(PrimitiveAnimation&&) noexcept = default;

    PrimitiveAnimation& PrimitiveAnimation::operator=(PrimitiveAnimation&&) noexcept = default;

    size_t PrimitiveAnimation::frame_count() const {
        return frames_.size();
    }

    float PrimitiveAnimation::frame_duration_seconds() const {
        return frame_duration_seconds_;
    }

    bool PrimitiveAnimation::loops() const {
        return loop_;
    }

    float PrimitiveAnimation::total_duration_seconds() const {
        return static_cast<float>(frames_.size()) * frame_duration_seconds_;
    }

    size_t PrimitiveAnimation::frame_index_at(const float elapsed_seconds) const {
        const size_t count = frames_.size();
        if (count == 0) {
            return 0;
        }
        if (frame_duration_seconds_ <= 0.f) {
            return count - 1;
        }

        if (elapsed_seconds <= 0.f) {
            return 0;
        }

        const float total = total_duration_seconds();

        if (loop_) {
            // Wrap around, like fmod, so playback repeats seamlessly.
            const float wrapped = std::fmod(elapsed_seconds, total);
            const auto index = static_cast<size_t>(wrapped / frame_duration_seconds_);
            return std::min(index, count - 1);
        }

        if (elapsed_seconds >= total) {
            return count - 1; // hold on the last frame once finished
        }

        const auto index = static_cast<size_t>(elapsed_seconds / frame_duration_seconds_);
        return std::min(index, count - 1);
    }

    const intrnl::Rect& PrimitiveAnimation::frame_at(const float elapsed_seconds) const {
        return rect_of(frame_index_at(elapsed_seconds));
    }

    const intrnl::Rect& PrimitiveAnimation::rect_of(const size_t index) const {
        if (index >= frames_.size()) {
            throw std::out_of_range("PrimitiveAnimation::rect_of - index out of range");
        }
        return frames_[index];
    }
}
