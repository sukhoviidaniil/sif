/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UI_WORLDVIEW_H
#define RENDER_ENGINE_UI_WORLDVIEW_H

#include "UIElement.h"

#include "sif/math/Point2.h"
#include "sif/render/Camera.h"

namespace sif::ui {
/**
 * @brief A panel of the UI that shows the game world through a Camera.
 *
 * The bridge between the two coordinate systems of the engine: the UI
 * works in screen pixels, the world in normalized units ([-1, 1]), and
 * rnd::Camera converts between them. A WorldView is laid out like any
 * other element (so a HUD can sit next to or on top of it, on a higher
 * layer), points its camera's viewport at its own rect, and places each
 * child at camera.world_to_screen(child->world_rect) - any element can
 * be a world object: a Sprite, an Animation, a Rectangle, even a whole
 * Overlay. Moving or zooming the camera moves everything in the view and
 * nothing outside it.
 *
 * Children whose world_rect is outside the camera's visible bounds are
 * culled (not drawn), and the view clips to its rect, so a world object
 * half off the panel never draws over the HUD next to it.
 */
class WorldView : public UIElement {
public:
    WorldView();
    ~WorldView() override = default;

    /// @brief Fills its slot (Auto) or takes its explicit size, like any viewport.
    math::Vector2 measure(const math::Vector2& available) override;

    /// @brief Points the camera at r and projects every child's world_rect into it.
    void layout(intrnl::Rect r) override;

    /// @brief Draws only the children the camera can see.
    void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const override;

    /// @brief A screen point (e.g. a click) in world coordinates.
    [[nodiscard]] math::Point2 to_world(math::Vector2 screen_point) const;

    /// @brief True if child is currently culled (its world_rect is out of view).
    [[nodiscard]] bool culled(const UIElement& child) const;

    /// @brief The projection; change center, zoom, aspect policy or y axis freely.
    rnd::Camera camera;
};
} // namespace sif::ui

#endif // RENDER_ENGINE_UI_WORLDVIEW_H
