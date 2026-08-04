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

    bool Menu::handle_click(const math::Vector2 point) {
        for (Button* b : buttons_) {
            if (b->contains(point)) {
                set_active(b->id);
                return true;
            }
        }
        return false;
    }
}
