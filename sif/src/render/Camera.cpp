/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-08-04
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/render/Camera.h"

#include <algorithm>
#include <cmath>

#include "sif/diagnostics/Logger.h"

namespace sif::rnd {
    namespace {
        /**
         * @brief Half-extent of the world along one axis, in world units.
         *
         * The world spans [-1, 1], so one unit from the centre to either
         * edge. Zoom shrinks what is visible, hence the division.
         */
        constexpr float world_half_extent = 1.f;

        bool is_degenerate(const intrnl::Rect& r) {
            return r.width <= 0.f || r.height <= 0.f;
        }
    }

    Camera::Camera() = default;

    Camera::Camera(const math::Vector2 screen_size, const AspectPolicy policy)
        : policy_(policy) {
        set_screen_size(screen_size);
    }

    void Camera::set_screen_size(const math::Vector2 screen_size) {
        screen_size_ = {std::max(0.f, screen_size.x), std::max(0.f, screen_size.y)};
        // Resizing the window resets the viewport to "all of it"; a
        // caller that wants a sub-region re-establishes it afterwards,
        // which is less surprising than silently keeping a viewport that
        // may now stick out of the screen.
        viewport_ = {0.f, 0.f, screen_size_.x, screen_size_.y};
    }

    math::Vector2 Camera::screen_size() const {
        return screen_size_;
    }

    void Camera::set_viewport(const intrnl::Rect viewport) {
        if (is_degenerate(viewport)) {
            LOG("Camera::set_viewport - ignoring a viewport with no area");
            return;
        }
        viewport_ = viewport;
    }

    const intrnl::Rect & Camera::viewport() const {
        return viewport_;
    }

    void Camera::set_aspect_policy(const AspectPolicy policy) {
        policy_ = policy;
    }

    AspectPolicy Camera::aspect_policy() const {
        return policy_;
    }

    void Camera::set_center(const math::Point2 world_center) {
        center_ = world_center;
    }

    math::Point2 Camera::center() const {
        return center_;
    }

    void Camera::set_zoom(const float zoom) {
        if (!(zoom > 0.f)) {
            // Also rejects NaN, which would poison every later
            // projection silently.
            LOG("Camera::set_zoom - zoom must be positive, keeping the previous value");
            return;
        }
        zoom_ = zoom;
    }

    float Camera::zoom() const {
        return zoom_;
    }

    void Camera::set_y_axis_up(const bool up) {
        y_axis_up_ = up;
    }

    bool Camera::y_axis_up() const {
        return y_axis_up_;
    }

    math::Vector2 Camera::scale() const {
        if (is_degenerate(viewport_)) {
            return {0.f, 0.f};
        }

        // Pixels available from the centre to each edge of the viewport.
        const float half_w = viewport_.width * 0.5f;
        const float half_h = viewport_.height * 0.5f;

        // Pixels per world unit if each axis were scaled on its own.
        const float sx = half_w / world_half_extent * zoom_;
        const float sy = half_h / world_half_extent * zoom_;

        switch (policy_) {
            case AspectPolicy::Stretch:
                return {sx, sy};
            case AspectPolicy::Fill:
                // Cover the viewport: the larger scale wins, the excess
                // falls outside and is cropped.
                return {std::max(sx, sy), std::max(sx, sy)};
            case AspectPolicy::Fit:
            default:
                // Keep everything visible: the smaller scale wins, the
                // remainder becomes empty margin.
                return {std::min(sx, sy), std::min(sx, sy)};
        }
    }

    math::Point2 Camera::viewport_center() const {
        return {
            viewport_.x + viewport_.width * 0.5f,
            viewport_.y + viewport_.height * 0.5f
        };
    }

    math::Point2 Camera::world_to_screen(const math::Point2 world_point) const {
        const math::Vector2 s = scale();
        const math::Point2 origin = viewport_center();

        const float dx = world_point.x - center_.x;
        const float dy = world_point.y - center_.y;

        // Screen y always grows downwards; a world whose y grows upwards
        // is therefore mirrored here and nowhere else.
        const float screen_dy = y_axis_up_ ? -dy : dy;

        return {origin.x + dx * s.x, origin.y + screen_dy * s.y};
    }

    math::Point2 Camera::screen_to_world(const math::Point2 screen_point) const {
        const math::Vector2 s = scale();
        if (s.x == 0.f || s.y == 0.f) {
            // No projection exists yet (no viewport), so the only honest
            // answer is the camera centre.
            return center_;
        }

        const math::Point2 origin = viewport_center();

        const float dx = (screen_point.x - origin.x) / s.x;
        const float screen_dy = (screen_point.y - origin.y) / s.y;
        const float dy = y_axis_up_ ? -screen_dy : screen_dy;

        return {center_.x + dx, center_.y + dy};
    }

    math::Vector2 Camera::world_to_screen_size(const math::Vector2 world_size) const {
        const math::Vector2 s = scale();
        return {std::abs(world_size.x) * s.x, std::abs(world_size.y) * s.y};
    }

    intrnl::Rect Camera::world_to_screen(const intrnl::Rect &world_rect) const {
        // Project both corners rather than "corner + size": that way the
        // y flip is handled by the same code path as points, instead of
        // being a special case that has to be remembered.
        const math::Point2 a = world_to_screen({world_rect.x, world_rect.y});
        const math::Point2 b = world_to_screen({
            world_rect.x + world_rect.width,
            world_rect.y + world_rect.height
        });

        return {
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::abs(b.x - a.x),
            std::abs(b.y - a.y)
        };
    }

    intrnl::Rect Camera::visible_world_bounds() const {
        const math::Vector2 s = scale();
        if (s.x == 0.f || s.y == 0.f) {
            return {center_.x, center_.y, 0.f, 0.f};
        }

        const float half_w = viewport_.width * 0.5f / s.x;
        const float half_h = viewport_.height * 0.5f / s.y;

        return {
            center_.x - half_w,
            center_.y - half_h,
            half_w * 2.f,
            half_h * 2.f
        };
    }

    bool Camera::is_visible(const intrnl::Rect &world_rect) const {
        const intrnl::Rect view = visible_world_bounds();
        if (is_degenerate(view)) {
            return false;
        }

        return world_rect.x < view.x + view.width
            && world_rect.x + world_rect.width > view.x
            && world_rect.y < view.y + view.height
            && world_rect.y + world_rect.height > view.y;
    }
}
