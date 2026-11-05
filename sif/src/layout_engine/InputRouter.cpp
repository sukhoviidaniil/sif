/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/InputRouter.h"

#include <algorithm>
#include <functional>
#include <typeindex>

namespace sif::ui {
namespace {
template <typename T>
const T* as(const event::EventConcept& ev) {
    return ev.type() == std::type_index(typeid(T)) ? static_cast<const T*>(ev.data()) : nullptr;
}

math::Vector2 point_of(const int x, const int y) { return {static_cast<float>(x), static_cast<float>(y)}; }

void set_flag(const std::vector<UIElement*>& path, bool UIElement::*flag, const bool value) {
    for (UIElement* e : path) {
        e->*flag = value;
    }
}
} // namespace

InputRouter::InputRouter(UIElement& root) : root_(root) {}

bool InputRouter::handle(const event::EventConcept& ev) {
    using namespace event::input;

    if (const auto* e = as<Mouse_Moved>(ev)) {
        return pointer_move(point_of(e->x, e->y));
    }
    if (const auto* e = as<Mouse_ButtonPressed>(ev)) {
        return pointer_down(point_of(e->x, e->y), e->button);
    }
    if (const auto* e = as<Mouse_ButtonReleased>(ev)) {
        return pointer_up(point_of(e->x, e->y), e->button);
    }
    if (const auto* e = as<Mouse_WheelScrolled>(ev)) {
        return wheel(point_of(e->x, e->y), e->delta);
    }
    if (const auto* e = as<KeyPressed>(ev)) {
        return key(e->key, e->shift);
    }
    if (const auto* e = as<TextEntered>(ev)) {
        return text(e->unicode);
    }
    return false;
}

bool InputRouter::dispatch(UIEvent& e, const std::vector<UIElement*>& path) {
    // Bubble: target first, then every ancestor, until one stops it.
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
        if ((*it)->on_ui_event(e)) {
            break;
        }
    }
    return e.handled;
}

bool InputRouter::pointer_move(const math::Vector2 point) {
    const std::vector<UIElement*> path = root_.hit_path(point);

    // hovered follows the path, like CSS :hover - a button stays hovered
    // while the pointer is over its label.
    set_flag(hover_path_, &UIElement::hovered, false);
    hover_path_ = path;
    set_flag(hover_path_, &UIElement::hovered, true);

    if (path.empty()) {
        return false;
    }
    UIEvent e{.type = UIEventType::PointerMove, .point = point, .target = path.back()};
    return dispatch(e, path);
}

bool InputRouter::pointer_down(const math::Vector2 point, const event::input::MouseButton button) {
    const std::vector<UIElement*> path = root_.hit_path(point);

    set_flag(press_path_, &UIElement::pressed, false);
    press_path_ = path;
    set_flag(press_path_, &UIElement::pressed, true);

    if (path.empty()) {
        return false;
    }
    UIEvent e{.type = UIEventType::PointerDown, .point = point, .button = button, .target = path.back()};
    return dispatch(e, path);
}

bool InputRouter::pointer_up(const math::Vector2 point, const event::input::MouseButton button) {
    const std::vector<UIElement*> path = root_.hit_path(point);
    const std::vector<UIElement*> pressed = std::move(press_path_);
    press_path_.clear();
    set_flag(pressed, &UIElement::pressed, false);

    bool handled = false;
    if (!path.empty()) {
        UIEvent up{.type = UIEventType::PointerUp, .point = point, .button = button, .target = path.back()};
        handled = dispatch(up, path);
    }

    if (button != event::input::MouseButton::Left) {
        return handled;
    }

    // Click: the deepest element under both the press and the release.
    std::size_t common = 0;
    while (common < path.size() && common < pressed.size() && path[common] == pressed[common]) {
        ++common;
    }
    if (common == 0) {
        return handled;
    }
    const std::vector<UIElement*> click_path(path.begin(), path.begin() + static_cast<std::ptrdiff_t>(common));

    // A completed click moves focus to what was clicked (or clears it
    // when nothing focusable was clicked). On the click, not on the
    // press: pressing a menu entry and dragging away must not change
    // anything, and a Menu's selection follows focus.
    UIElement* focus_target = nullptr;
    for (auto it = click_path.rbegin(); it != click_path.rend(); ++it) {
        if ((*it)->can_focus()) {
            focus_target = *it;
            break;
        }
    }
    focus(focus_target);
    UIEvent click{.type = UIEventType::Click, .point = point, .button = button, .target = click_path.back()};
    return dispatch(click, click_path) || handled;
}

bool InputRouter::wheel(const math::Vector2 point, const float delta) {
    const std::vector<UIElement*> path = root_.hit_path(point);
    if (path.empty()) {
        return false;
    }
    UIEvent e{.type = UIEventType::Wheel, .point = point, .wheel_delta = delta, .target = path.back()};
    return dispatch(e, path);
}

bool InputRouter::key(const event::input::Key k, const bool shift) {
    using event::input::Key;

    if (k == Key::Tab && focused_ == nullptr) {
        return move_focus(shift ? -1 : +1); // Tab is the way *into* a UI
    }
    if (focused_ == nullptr) {
        return false; // nothing focused: the keyboard belongs to the game
    }

    const std::vector<UIElement*> path = path_to(focused_);
    UIEvent e{.type = UIEventType::Key, .key = k, .target = focused_};
    if (dispatch(e, path)) {
        return true;
    }

    switch (k) {
    case Key::Enter:
    case Key::Space: {
        UIEvent click{.type = UIEventType::Click, .target = focused_};
        return dispatch(click, path);
    }
    case Key::Tab:
        return move_focus(shift ? -1 : +1);
    case Key::Down:
    case Key::Right:
        move_focus(+1);
        return true;
    case Key::Up:
    case Key::Left:
        move_focus(-1);
        return true;
    default:
        return false;
    }
}

bool InputRouter::text(const char32_t character) {
    if (focused_ == nullptr) {
        return false;
    }
    UIEvent e{.type = UIEventType::Text, .character = character, .target = focused_};
    return dispatch(e, path_to(focused_));
}

void InputRouter::focus(UIElement* element) {
    if (element == focused_) {
        return;
    }
    if (focused_ != nullptr) {
        UIElement* previous = focused_;
        previous->focused = false;
        focused_ = nullptr;
        UIEvent out{.type = UIEventType::FocusOut, .target = previous};
        dispatch(out, path_to(previous));
    }
    if (element != nullptr) {
        focused_ = element;
        element->focused = true;
        UIEvent in{.type = UIEventType::FocusIn, .target = element};
        dispatch(in, path_to(element));
    }
}

UIElement* InputRouter::focused() const { return focused_; }

void InputRouter::reset() {
    set_flag(hover_path_, &UIElement::hovered, false);
    set_flag(press_path_, &UIElement::pressed, false);
    hover_path_.clear();
    press_path_.clear();
    if (focused_ != nullptr) {
        focused_->focused = false;
        focused_ = nullptr;
    }
}

std::vector<UIElement*> InputRouter::path_to(const UIElement* element) const {
    std::vector<UIElement*> path;
    const std::function<bool(UIElement&)> walk = [&](UIElement& e) {
        path.push_back(&e);
        if (&e == element) {
            return true;
        }
        for (auto& c : e.children) {
            if (walk(*c)) {
                return true;
            }
        }
        path.pop_back();
        return false;
    };
    walk(root_);
    return path;
}

std::vector<UIElement*> InputRouter::focusables() const {
    std::vector<UIElement*> out;
    const std::function<void(UIElement&)> walk = [&](UIElement& e) {
        if (!e.visible) {
            return; // nothing inside a hidden subtree can take focus
        }
        if (e.can_focus()) {
            out.push_back(&e);
        }
        for (auto& c : e.children) {
            walk(*c);
        }
    };
    walk(root_);
    return out;
}

bool InputRouter::move_focus(const int step) {
    const std::vector<UIElement*> all = focusables();
    if (all.empty()) {
        return false;
    }
    const auto it = std::find(all.begin(), all.end(), focused_);
    if (it == all.end()) {
        focus(step < 0 ? all.back() : all.front());
        return true;
    }
    const auto n = static_cast<std::ptrdiff_t>(all.size());
    const std::ptrdiff_t index = ((it - all.begin()) + step + n) % n; // wraps around
    focus(all[static_cast<std::size_t>(index)]);
    return true;
}
} // namespace sif::ui
