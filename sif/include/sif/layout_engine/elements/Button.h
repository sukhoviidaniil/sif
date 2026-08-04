/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_BUTTON_H
#define RENDER_ENGINE_BUTTON_H

#include "UIElement.h"
#include "sif/internal/Color.h"
#include "sif/internal/RecordID.h"

namespace sif::ui {
    /**
     * @brief The two states a Button can be in.
     *
     * State pattern: rather than scattering "if selected then... else
     * ..." branches across measure/render/click-handling, Button's
     * behavior in each state is defined by a small, explicit set of
     * values (ButtonState + the two *_color fields below) that the
     * class interpolates between. This keeps adding a third state (if
     * ever needed - e.g. Disabled) a matter of extending the enum and
     * the color table, not hunting down every branch.
     */
    enum class ButtonState {
        Unselected,
        Selected
    };

    /**
     * @brief Clickable UI element with two states and a smooth transition.
     *
     * Selecting/unselecting does not flip instantly: transition_
     * (0 = fully Unselected, 1 = fully Selected) is animated over
     * transition_duration_seconds by update(), and append_render_items
     * blends unselected_color/selected_color by that factor - so the
     * button visibly eases between states instead of snapping.
     *
     * Each Button carries its own id (UIElement::id), which is how a
     * containing ui::Menu identifies and activates a specific button.
     */
    class Button : public UIElement {
    public:
        Button() = default;

        math::Vector2 measure(const math::Vector2& available) override;
        void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const override;

        /**
         * @brief Advances the smooth Selected/Unselected transition by dt seconds.
         */
        void update(float dt) override;

        /**
         * @brief Returns true if point falls within this button's laid-out rect.
         *
         * Uses result.rect, so this is only meaningful after layout()
         * has run at least once.
         */
        [[nodiscard]] bool contains(math::Vector2 point) const;

        /**
         * @brief Hit-tests point and toggles Selected/Unselected if it hits.
         *
         * Useful for a Button used standalone (not inside a Menu, whose
         * set_active() enforces single-selection instead - see Menu.h).
         *
         * @return true if point was within this button's rect (and the
         * state was toggled), false otherwise.
         */
        bool handle_click(math::Vector2 point);

        /// @brief Immediately requests the Selected state (still eases in visually).
        void select();

        /// @brief Immediately requests the Unselected state (still eases out visually).
        void unselect();

        [[nodiscard]] bool is_selected() const;

        /// @brief How long the smooth Selected<->Unselected transition takes, in seconds.
        float transition_duration_seconds = 0.15f;

        intrnl::Color unselected_color = intrnl::Color(80, 80, 80);
        intrnl::Color selected_color = intrnl::Color(80, 160, 255);

    private:
        ButtonState state_ = ButtonState::Unselected;
        float transition_ = 0.f; ///< 0 = fully Unselected, 1 = fully Selected
    };
}

#endif //RENDER_ENGINE_BUTTON_H
