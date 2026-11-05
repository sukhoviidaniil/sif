/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-07-06
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/Menu.h"

#include <algorithm>

namespace sif::ui {
void Menu::add(std::unique_ptr<UIElement> child) {
    if (auto* button = dynamic_cast<Button*>(child.get())) {
        buttons_.push_back(button);
    }
    UIElement::add(std::move(child));
}

Button& Menu::add_button(std::unique_ptr<Button> button) {
    Button& ref = *button;
    add(std::move(button));
    return ref;
}

std::vector<intrnl::RecordID> Menu::button_ids() const {
    std::vector<intrnl::RecordID> ids;
    ids.reserve(buttons_.size());
    for (const Button* b : buttons_) {
        ids.push_back(b->id);
    }
    return ids;
}

void Menu::set_active(const intrnl::RecordID button_id) {
    bool found = false;
    for (Button* b : buttons_) {
        if (b->id == button_id) {
            found = true;
            break;
        }
    }
    if (!found) {
        return; // unknown id: leave the menu's selection untouched
    }

    for (Button* b : buttons_) {
        if (b->id == button_id) {
            b->select();
        } else {
            b->unselect();
        }
    }
}

std::optional<intrnl::RecordID> Menu::active_button_id() const {
    for (const Button* b : buttons_) {
        if (b->is_selected()) {
            return b->id;
        }
    }
    return std::nullopt;
}

namespace {
/// @brief True if target is root or one of its descendants.
bool within(const UIElement& root, const UIElement* target) {
    if (&root == target) {
        return true;
    }
    return std::any_of(root.children.begin(), root.children.end(),
                       [target](const std::unique_ptr<UIElement>& c) { return within(*c, target); });
}
} // namespace

bool Menu::on_ui_event(UIEvent& e) {
    if ((e.type == UIEventType::Click || e.type == UIEventType::FocusIn) && e.target != nullptr) {
        for (Button* b : buttons_) {
            if (b->enabled && within(*b, e.target)) {
                set_active(b->id);
                if (e.type == UIEventType::Click) {
                    e.handled = true;
                }
                break;
            }
        }
    }
    return UIElement::on_ui_event(e);
}

bool Menu::handle_click(const math::Vector2 point) {
    // Ask the paint order, not the button list: with overlapping
    // buttons (z_index, an Overlay or negative spacing inside the
    // menu) the first button in the list whose rect contains the
    // point is not necessarily the one drawn on top, and a non-button
    // element painted over a button must block the click instead of
    // passing it through. Hidden buttons are also no longer clickable,
    // since element_at skips invisible subtrees.
    Button* hit = element_at<Button>(point);
    if (hit == nullptr) {
        return false;
    }
    // A Button nested inside some other child of the menu is not one
    // of the menu's own (see add()), so it does not take part in the
    // menu's single selection.
    if (std::find(buttons_.begin(), buttons_.end(), hit) == buttons_.end()) {
        return false;
    }
    set_active(hit->id);
    return true;
}
} // namespace sif::ui
