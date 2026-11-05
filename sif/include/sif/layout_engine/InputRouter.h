/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UI_INPUTROUTER_H
#define RENDER_ENGINE_UI_INPUTROUTER_H

#include <vector>

#include "sif/event/Event.h"
#include "sif/layout_engine/UIEvent.h"
#include "sif/layout_engine/elements/UIElement.h"

namespace sif::ui {
/**
 * @brief Turns raw input events into UI events on a UI tree.
 *
 * Mediator between the platform's input and the element tree: elements
 * never see event::input::* and never track the pointer or focus
 * themselves; the router does, once, and tells each element what
 * happened to it through UIElement::on_ui_event.
 *
 *  - Mouse_Moved      -> updates `hovered` on the path under the pointer,
 *                        PointerMove to the element under it.
 *  - Mouse_ButtonPressed -> PointerDown, `pressed` on the path.
 *  - Mouse_ButtonReleased -> PointerUp, and for the left button a Click
 *                        on the deepest element that was under both the
 *                        press and the release (pressing on one button
 *                        and releasing over another clicks neither).
 *                        A completed click also moves keyboard focus to
 *                        the deepest focusable element it hit (or clears
 *                        it) - on the click, not the press, so a press
 *                        that is dragged away changes nothing.
 *  - Mouse_WheelScrolled -> Wheel to the element under the pointer.
 *  - KeyPressed (Tab) -> focus to the next focusable element (Shift+Tab:
 *                        previous) - also when nothing has focus yet, as
 *                        the conventional way into a UI. Not used (so left
 *                        to the game) when the tree has nothing focusable.
 *  - KeyPressed       -> Key to the focused element; if nobody handles it,
 *                        Enter/Space activate (Click) the focused element
 *                        and the arrow keys move focus to the previous/next
 *                        focusable element in document order. Arrows only
 *                        move focus while something *has* focus, so a game
 *                        that steers with the arrow keys keeps them.
 *  - TextEntered      -> Text to the focused element.
 *
 * Hit-testing is UIElement::hit_path, so layers, z_index, clipping and
 * hit_testable all apply to input exactly as they do to drawing.
 *
 * The router keeps non-owning pointers into the tree (hover path,
 * pressed path, focus). Call reset() after removing elements from it.
 */
class InputRouter {
public:
    explicit InputRouter(UIElement& root);

    /**
     * @brief Routes one raw event.
     *
     * @return true if the UI used it (some handler set UIEvent::handled),
     * so the caller can decide not to pass it on to game logic.
     */
    bool handle(const event::EventConcept& ev);

    /// @brief Moves keyboard focus (nullptr clears it), sending FocusOut/FocusIn.
    void focus(UIElement* element);

    [[nodiscard]] UIElement* focused() const;

    /// @brief Forgets hover/press/focus state, e.g. after the tree was changed.
    void reset();

private:
    bool pointer_move(math::Vector2 point);
    bool pointer_down(math::Vector2 point, event::input::MouseButton button);
    bool pointer_up(math::Vector2 point, event::input::MouseButton button);
    bool wheel(math::Vector2 point, float delta);
    bool key(event::input::Key key, bool shift);
    bool text(char32_t character);

    /// @brief Sends e to e.target and then up `path` (root first, target last) until stopped.
    static bool dispatch(UIEvent& e, const std::vector<UIElement*>& path);

    /// @brief Root-first path to element, or empty if it is not in the tree.
    [[nodiscard]] std::vector<UIElement*> path_to(const UIElement* element) const;

    /// @brief Every visible focusable element, in document order.
    [[nodiscard]] std::vector<UIElement*> focusables() const;

    /// @return false if there is nothing focusable to move to.
    bool move_focus(int step);

    UIElement& root_;
    std::vector<UIElement*> hover_path_;
    std::vector<UIElement*> press_path_;
    UIElement* focused_ = nullptr;
};
} // namespace sif::ui

#endif // RENDER_ENGINE_UI_INPUTROUTER_H
