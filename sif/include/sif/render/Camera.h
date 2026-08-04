/***************************************************************
* Project:          Render_Engine
* File:             Camera.h
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-08-04
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_CAMERA_H
#define RENDER_ENGINE_CAMERA_H

#include "sif/internal/Rect.h"
#include "sif/math/Point2.h"
#include "sif/math/Vector2.h"

namespace sif::rnd {

    /**
     * @brief How the world is fitted into a viewport whose aspect ratio
     * does not match it.
     *
     * The world is square by definition (both axes span [-1, 1]), while
     * a window almost never is, so *something* has to give. Making that
     * an explicit choice rather than an accident is the whole reason
     * this enum exists.
     */
    enum class AspectPolicy {
        /**
         * @brief Fill the viewport exactly, scaling each axis on its own.
         *
         * Nothing is cropped and no bars appear, but a square in the
         * world is no longer square on screen. Fine for backgrounds and
         * UI-ish content, wrong for a tile grid.
         */
        Stretch,

        /**
         * @brief Uniform scale, whole world visible, bars on the sides.
         *
         * The safe default for a game board: every tile stays square and
         * nothing can be pushed off-screen by an unusual window shape.
         */
        Fit,

        /**
         * @brief Uniform scale, viewport fully covered, edges cropped.
         *
         * Use when the world must reach every pixel and losing its
         * margins is acceptable.
         */
        Fill
    };

    /**
     * @brief Projects normalized world coordinates onto window pixels.
     *
     * The world is modelled in a resolution-independent, normalized
     * coordinate system bounded by [-1, 1] on both axes. Game logic
     * therefore never knows how large the window is - it can be resized,
     * moved to a different display, or (in principle) rendered to a text
     * terminal, and not one line of logic changes. Turning those
     * normalized positions into pixels is this class's only job.
     *
     * Everything is computed manually from the viewport, the aspect
     * policy, the zoom and the camera centre; no graphics library is
     * involved, which is what lets this header live in the engine and
     * be unit-tested without a window.
     *
     * @par Axis direction
     * By default y grows *downwards*, matching intrnl::Rect, the layout
     * engine and every screen coordinate system in the project, so
     * world (-1, -1) is the top-left corner. Set y_axis_up if you prefer
     * the mathematical convention; the projection flips accordingly and
     * nothing else in the class changes.
     *
     * @par Typical use
     * @code
     * rnd::Camera camera({800.f, 600.f}, rnd::AspectPolicy::Fit);
     * // ... on Window_Resized:
     * camera.set_screen_size({new_w, new_h});
     * // ... when building a render item for an entity:
     * item->rect = camera.world_to_screen(entity.bounds());
     * @endcode
     */
    class Camera {
    public:
        Camera();

        /**
         * @brief Constructs a camera targeting the whole screen.
         *
         * @param screen_size Size of the render target, in pixels.
         * @param policy How to reconcile a non-square viewport with the
         * square world.
         */
        explicit Camera(math::Vector2 screen_size, AspectPolicy policy = AspectPolicy::Fit);

        // ===== Target =====

        /**
         * @brief Sets the render target size and resets the viewport to
         * cover all of it.
         *
         * Call this from the window-resize handler; the projection
         * adapts, the world does not move.
         */
        void set_screen_size(math::Vector2 screen_size);

        [[nodiscard]] math::Vector2 screen_size() const;

        /**
         * @brief Restricts drawing to a sub-rectangle of the screen.
         *
         * Lets the same world be projected into a panel (split screen, a
         * minimap, a board next to a HUD column) instead of the whole
         * window. Degenerate (zero or negative) rectangles are ignored.
         */
        void set_viewport(intrnl::Rect viewport);

        [[nodiscard]] const intrnl::Rect& viewport() const;

        // ===== Framing =====

        void set_aspect_policy(AspectPolicy policy);
        [[nodiscard]] AspectPolicy aspect_policy() const;

        /**
         * @brief Sets the world point that appears at the viewport centre.
         *
         * Defaults to the world origin, which for a static board is all
         * you ever need; a scrolling game moves this instead of moving
         * every entity.
         */
        void set_center(math::Point2 world_center);

        [[nodiscard]] math::Point2 center() const;

        /**
         * @brief Sets the zoom factor (1 = the whole [-1, 1] world fits).
         *
         * Values above 1 magnify. Non-positive values are rejected
         * (they would collapse the projection and make screen_to_world
         * undefined), so the previous zoom is kept instead.
         */
        void set_zoom(float zoom);

        [[nodiscard]] float zoom() const;

        /// @brief Whether world y grows upwards (false by default: y grows down).
        void set_y_axis_up(bool up);
        [[nodiscard]] bool y_axis_up() const;

        // ===== Projection =====

        /**
         * @brief Pixels per one world unit, per axis.
         *
         * Under Fit/Fill both components are equal; under Stretch they
         * differ. Multiply a world size by this to get a sprite's pixel
         * size.
         */
        [[nodiscard]] math::Vector2 scale() const;

        /// @brief Projects a world point to a pixel position.
        [[nodiscard]] math::Point2 world_to_screen(math::Point2 world_point) const;

        /// @brief The exact inverse of world_to_screen.
        [[nodiscard]] math::Point2 screen_to_world(math::Point2 screen_point) const;

        /**
         * @brief Converts a world-space size to a pixel size.
         *
         * Always non-negative, even when the y axis is flipped: a height
         * is a magnitude, not a direction.
         */
        [[nodiscard]] math::Vector2 world_to_screen_size(math::Vector2 world_size) const;

        /**
         * @brief Projects an axis-aligned world rectangle.
         *
         * The result is normalised so that x/y are always the top-left
         * corner on screen and width/height are non-negative, whichever
         * way the y axis points.
         */
        [[nodiscard]] intrnl::Rect world_to_screen(const intrnl::Rect& world_rect) const;

        /**
         * @brief The part of the world currently visible in the viewport.
         *
         * Useful for culling: an entity whose bounds do not intersect
         * this rectangle cannot contribute a visible pixel, so it need
         * not produce a render item at all.
         */
        [[nodiscard]] intrnl::Rect visible_world_bounds() const;

        /// @brief True if the world rectangle overlaps visible_world_bounds().
        [[nodiscard]] bool is_visible(const intrnl::Rect& world_rect) const;

    private:
        /// @brief Centre of the viewport, in pixels.
        [[nodiscard]] math::Point2 viewport_center() const;

        math::Vector2 screen_size_{0.f, 0.f};
        intrnl::Rect viewport_{};
        AspectPolicy policy_ = AspectPolicy::Fit;
        math::Point2 center_{0.f, 0.f};
        float zoom_ = 1.f;
        bool y_axis_up_ = false;
    };
}

#endif //RENDER_ENGINE_CAMERA_H
