/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UI_SCROLLVIEW_H
#define RENDER_ENGINE_UI_SCROLLVIEW_H

#include "UIElement.h"

#include "sif/internal/Rect.h"
#include "sif/math/Vector2.h"

namespace sif::ui {
/// @brief Which axes a ScrollView scrolls along.
enum class ScrollDirection { Vertical, Horizontal, Both };

/**
 * @brief A viewport onto content larger than itself.
 *
 * The content (its children - usually one VBox/HBox) is offered
 * unlimited space along the scroll axis, so it measures to its full,
 * natural length; the ScrollView then shows the part of it under the
 * current scroll offset, clipped to its own rect (UIElement::clip is on
 * by default). What is scrolled out is neither drawn nor clickable.
 *
 * Content children should have a definite size along the scroll axis
 * (PX, or containers of such children): an Auto leaf would ask for the
 * unlimited space it was offered, so it is given the viewport's length
 * instead.
 */
class ScrollView : public UIElement {
public:
    ScrollView();
    ~ScrollView() override = default;

    /**
     * @brief Fills its slot (Auto) or takes its explicit size - never
     * the content's size, which is the whole point of a viewport.
     */
    math::Vector2 measure(const math::Vector2& available) override;

    /// @brief Lays the content out at its natural size, shifted by the scroll offset.
    void layout(intrnl::Rect r) override;

    /// @brief Moves the view by delta (positive = towards the end), clamped to the content.
    void scroll_by(math::Vector2 delta);

    /// @brief Moves the view to an absolute offset, clamped to the content.
    void scroll_to(math::Vector2 target);

    /**
     * @brief Scrolls by one mouse-wheel step along the primary axis.
     *
     * @param wheel_delta As reported by Mouse_WheelScrolled (positive =
     * wheel moved up/away from the user, which scrolls towards the
     * start, as in every desktop UI).
     * @return true if the view actually moved (false at either end).
     */
    bool handle_wheel(float wheel_delta);

    /**
     * @brief Scrolls on a Wheel UIEvent.
     *
     * Stops the event only if the view actually moved: at either end the
     * wheel bubbles on, so an outer ScrollView takes over - nested lists
     * scroll the way users expect.
     */
    bool on_ui_event(UIEvent& e) override;

    [[nodiscard]] math::Vector2 scroll_offset() const;

    /// @brief Size of the content as of the last layout().
    [[nodiscard]] math::Vector2 content_size() const;

    /// @brief Largest valid scroll offset on each axis (0 when the content fits).
    [[nodiscard]] math::Vector2 max_scroll() const;

    /// @brief The visible area (result.rect minus padding).
    [[nodiscard]] intrnl::Rect viewport() const;

    ScrollDirection direction = ScrollDirection::Vertical;

    /// @brief Pixels scrolled per mouse-wheel notch.
    float scroll_step = 40.f;

    /**
     * @brief Stay at the end while the content grows (a log or a chat).
     *
     * If the view is at its end when layout() finds the content has
     * grown, it moves to the new end; scrolled anywhere else, it stays
     * put, so reading older lines is not interrupted.
     */
    bool follow_end = false;

private:
    [[nodiscard]] bool scrolls_x() const;
    [[nodiscard]] bool scrolls_y() const;
    void clamp_offset();

    math::Vector2 offset_{0.f, 0.f};
    math::Vector2 content_size_{0.f, 0.f};
};
} // namespace sif::ui

#endif // RENDER_ENGINE_UI_SCROLLVIEW_H
