/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UI_UIEVENT_H
#define RENDER_ENGINE_UI_UIEVENT_H

#include "sif/event/events/input_Keyboard.h"
#include "sif/event/events/input_Mouse.h"
#include "sif/math/Vector2.h"

namespace sif::ui {
class UIElement;

/**
 * @brief What happened, from the UI's point of view.
 *
 * Raw input (event::input::*) says "the left button went down at
 * (x, y)"; a UIEvent says "this element was clicked". InputRouter does
 * that translation - see it for when each type is produced.
 */
enum class UIEventType {
    PointerMove, ///< The pointer moved (sent to what is now under it)
    PointerDown, ///< A mouse button went down over the target
    PointerUp,   ///< A mouse button went up over the target
    Click,       ///< Left button pressed and released over the same element, or Enter/Space on the focused one
    Wheel,       ///< Mouse wheel over the target
    Key,         ///< Key press, sent to the focused element
    Text,        ///< Typed character, sent to the focused element
    FocusIn,     ///< The target just received keyboard focus
    FocusOut     ///< The target just lost keyboard focus
};

/**
 * @brief One UI event travelling up the tree.
 *
 * Delivered first to `target` (the deepest element it is about), then
 * to each ancestor in turn - DOM-style bubbling, i.e. a Chain of
 * Responsibility over the element path - until a handler asks to stop.
 * Handlers set `handled` when they *used* the event (InputRouter
 * reports it, so the game knows the UI consumed that click or key) and
 * return true from UIElement::on_ui_event only to stop it from reaching
 * ancestors. The two are separate on purpose: a Button uses a click
 * (handled) but lets it bubble, so the Menu around it can react too.
 */
struct UIEvent {
    UIEventType type = UIEventType::PointerMove;

    /// Pointer position (pointer events and Wheel), in layout coordinates.
    math::Vector2 point{0.f, 0.f};
    event::input::MouseButton button = event::input::MouseButton::Left;
    float wheel_delta = 0.f;
    event::input::Key key = event::input::Key::Unknown;
    char32_t character = 0;

    /// The deepest element the event is about.
    UIElement* target = nullptr;

    /// Set by any handler that acted on the event.
    bool handled = false;
};
} // namespace sif::ui

#endif // RENDER_ENGINE_UI_UIEVENT_H
