/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-07-06
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_MENU_H
#define RENDER_ENGINE_MENU_H

#include <optional>
#include <vector>

#include "Button.h"
#include "LinearLayout.h"

namespace sif::ui {
/**
 * @brief A list of mutually-exclusive Buttons (a simple radio-button group).
 *
 * Mediator-flavored coordination: enforcing "exactly one button
 * active at a time" is the group's responsibility, not any single
 * Button's - a Button knows nothing about its siblings. Menu is
 * the single place that owns this invariant (see set_active), so
 * Button stays reusable standalone (see Button::handle_click) as
 * well as inside a Menu.
 *
 * Menu is a vertical LinearLayout by default (buttons stack top to
 * bottom); use align/spacing from LinearLayout to customize.
 */
class Menu : public LinearLayout {
public:
    Menu() = default;

    /**
     * @brief Adds a child element; Button children are additionally
     * tracked for id lookup / exclusivity (see set_active).
     *
     * Overridden (rather than only offering a separate add_button())
     * so this works uniformly whether a Button is added
     * programmatically or parsed from UI XML as a child <Button>
     * node (which goes through this same UIElement::add path via
     * UIFactory::build).
     */
    void add(std::unique_ptr<UIElement> child) override;

    /**
     * @brief Convenience wrapper around add() that returns a
     * reference to the just-added button.
     */
    Button& add_button(std::unique_ptr<Button> button);

    /**
     * @brief Returns the ids of every button currently in the menu,
     * in the order they were added.
     */
    [[nodiscard]] std::vector<intrnl::RecordID> button_ids() const;

    /**
     * @brief Selects the button with the given id and unselects all others.
     *
     * No-op if no button with that id exists in this menu.
     */
    void set_active(intrnl::RecordID button_id);

    /**
     * @brief Returns the id of the currently active (selected) button, if any.
     */
    [[nodiscard]] std::optional<intrnl::RecordID> active_button_id() const;

    /**
     * @brief Hit-tests point and activates the button drawn on top there.
     *
     * Resolved through UIElement::element_at, so overlapping buttons
     * pick the one actually on top (z_index / layer / document
     * order), and an element covering a button blocks the click.
     *
     * @return true if the topmost thing under point was one of this
     * menu's buttons (which was then made the active one), false
     * otherwise.
     */
    bool handle_click(math::Vector2 point);

    /**
     * @brief Keeps the selection in step with input routed by ui::InputRouter.
     *
     * A Click on (or inside) one of the menu's enabled buttons, and
     * keyboard focus arriving on one, make that button the active one -
     * so a menu driven by the mouse and one driven by the arrow keys
     * behave the same, without the game wiring either.
     */
    bool on_ui_event(UIEvent& e) override;

protected:
    [[nodiscard]] bool horizontal() const override { return false; }

private:
    std::vector<Button*> buttons_; ///< Non-owning; ownership lives in UIElement::children
};
} // namespace sif::ui

#endif // RENDER_ENGINE_MENU_H
