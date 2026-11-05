/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-11-05
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"

#include <algorithm>
#include <memory>
#include <string_view>
#include <string>
#include <vector>

#include "sif/event/Event.h"
#include "sif/event/Event_Store.h"
#include "sif/event/events/input_Keyboard.h"
#include "sif/event/events/input_Mouse.h"
#include "sif/layout_engine/InputRouter.h"
#include "sif/layout_engine/elements/Button.h"
#include "sif/layout_engine/elements/Menu.h"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/layout_engine/elements/ScrollView.h"
#include "sif/layout_engine/elements/V_HBox.h"
#include "sif/render/Layer.h"

using namespace sif;
using event::input::Key;
using event::input::MouseButton;

namespace {
    /// @brief Routes one raw event through router (via the same type-erased path App uses).
    template<typename Event>
    bool send(ui::InputRouter& router, Event e) {
        event::Event_Store store;
        store.push(std::move(e));
        const auto concept_ = store.pop_concept();
        return router.handle(*concept_);
    }

    bool click(ui::InputRouter& router, const float x, const float y) {
        const int ix = static_cast<int>(x), iy = static_cast<int>(y);
        const bool down = send(router, event::input::Mouse_ButtonPressed{MouseButton::Left, ix, iy});
        const bool up = send(router, event::input::Mouse_ButtonReleased{MouseButton::Left, ix, iy});
        return down || up;
    }

    bool key(ui::InputRouter& router, const Key k) {
        return send(router, event::input::KeyPressed{k, false, false, false, false});
    }

    /**
     * @brief A Menu with three 100x30 buttons stacked at y = 0, 30, 60
     * (the third disabled), a fourth button outside the menu at y = 90,
     * and a counter of on_click calls per button.
     */
    struct Ui {
        ui::VBox root;
        ui::Menu* menu = nullptr;
        std::vector<ui::Button*> buttons;
        std::vector<int> clicks = std::vector<int>(4, 0);

        Ui() {
            auto m = std::make_unique<ui::Menu>();
            menu = m.get();
            root.add(std::move(m));
            for (std::uint32_t i = 0; i < 4; ++i) {
                auto b = std::make_unique<ui::Button>();
                b->id = intrnl::RecordID(i + 1);
                b->name = "b" + std::to_string(i);
                b->width = intrnl::Size::px(100.f);
                b->height = intrnl::Size::px(30.f);
                b->on_click = [this, i](ui::Button&) { ++clicks[i]; };
                buttons.push_back(b.get());
                if (i < 3) {
                    menu->add_button(std::move(b));
                } else {
                    root.add(std::move(b));
                }
            }
            buttons[2]->enabled = false;
            ui::UIElement& as_element = root; // LinearLayout's layout API is protected
            as_element.measure({800.f, 600.f});
            as_element.layout({0.f, 0.f, 800.f, 600.f});
        }
    };
}

SIF_TEST(router_tracks_hover_along_the_path) {
    Ui ui;
    ui::InputRouter router(ui.root);

    send(router, event::input::Mouse_Moved{10, 40});
    SIF_CHECK(ui.buttons[1]->hovered && ui.menu->hovered && ui.root.hovered);
    SIF_CHECK(!ui.buttons[0]->hovered);

    send(router, event::input::Mouse_Moved{10, 5});
    SIF_CHECK(ui.buttons[0]->hovered && !ui.buttons[1]->hovered);

    send(router, event::input::Mouse_Moved{700, 500}); // over the root only
    SIF_CHECK(!ui.buttons[0]->hovered && ui.root.hovered && !ui.menu->hovered);
}

SIF_TEST(router_clicks_activate_buttons_and_menus_once) {
    Ui ui;
    ui::InputRouter router(ui.root);

    SIF_CHECK(click(router, 10, 40));
    SIF_CHECK(ui.clicks[1] == 1);
    SIF_CHECK(ui.menu->active_button_id() == intrnl::RecordID(2)); // the Menu saw the bubbling click
    SIF_CHECK(router.focused() == ui.buttons[1]);                  // and clicking focused it
    SIF_CHECK(!ui.buttons[1]->pressed);                             // released again

    // Pressing one button and releasing over another clicks neither.
    send(router, event::input::Mouse_ButtonPressed{MouseButton::Left, 10, 5});
    SIF_CHECK(ui.buttons[0]->pressed);
    send(router, event::input::Mouse_ButtonReleased{MouseButton::Left, 10, 40});
    SIF_CHECK(ui.clicks[0] == 0 && ui.clicks[1] == 1);

    // A disabled button neither clicks nor takes focus.
    SIF_CHECK(!click(router, 10, 70));
    SIF_CHECK(ui.clicks[2] == 0);
    SIF_CHECK(ui.menu->active_button_id() == intrnl::RecordID(2));
    SIF_CHECK(router.focused() == nullptr);

    // A click on nothing interactive is not used by the UI.
    SIF_CHECK(!click(router, 700, 500));
}

SIF_TEST(router_keyboard_focus_navigation_and_activation) {
    Ui ui;
    ui::InputRouter router(ui.root);

    // Nothing focused: arrows and Enter belong to the game.
    SIF_CHECK(!key(router, Key::Down));
    SIF_CHECK(!key(router, Key::Enter));

    router.focus(ui.buttons[0]);
    SIF_CHECK(ui.buttons[0]->focused);
    SIF_CHECK(ui.menu->active_button_id() == intrnl::RecordID(1)); // selection follows focus

    SIF_CHECK(key(router, Key::Down));
    SIF_CHECK(router.focused() == ui.buttons[1] && !ui.buttons[0]->focused);
    SIF_CHECK(key(router, Key::Down)); // skips the disabled button
    SIF_CHECK(router.focused() == ui.buttons[3]);
    SIF_CHECK(key(router, Key::Down)); // wraps around
    SIF_CHECK(router.focused() == ui.buttons[0]);
    SIF_CHECK(key(router, Key::Up));
    SIF_CHECK(router.focused() == ui.buttons[3]);

    SIF_CHECK(key(router, Key::Enter));
    SIF_CHECK(key(router, Key::Space));
    SIF_CHECK(ui.clicks[3] == 2);

    // Hidden elements cannot be reached.
    ui.buttons[1]->visible = false;
    router.focus(ui.buttons[0]);
    key(router, Key::Down);
    SIF_CHECK(router.focused() == ui.buttons[3]);

    // Other keys are left alone unless the focused element wants them.
    SIF_CHECK(!key(router, Key::W));
    ui.buttons[3]->event_handler = [](ui::UIEvent& e) {
        if (e.type == ui::UIEventType::Key && e.key == Key::W) {
            e.handled = true;
        }
        return false;
    };
    SIF_CHECK(key(router, Key::W));

    // Typed text goes to the focused element.
    char32_t typed = 0;
    ui.buttons[3]->event_handler = [&typed](ui::UIEvent& e) {
        if (e.type == ui::UIEventType::Text) {
            typed = e.character;
            e.handled = true;
        }
        return false;
    };
    SIF_CHECK(send(router, event::input::TextEntered{U'ї'}));
    SIF_CHECK(typed == U'ї');
}

SIF_TEST(router_events_bubble_until_a_handler_stops_them) {
    Ui ui;
    ui::InputRouter router(ui.root);
    std::vector<std::string> seen;
    ui.menu->event_handler = [&seen](ui::UIEvent& e) {
        if (e.type == ui::UIEventType::PointerDown) {
            seen.push_back("menu");
        }
        return true; // stop here
    };
    ui.root.event_handler = [&seen](ui::UIEvent& e) {
        if (e.type == ui::UIEventType::PointerDown) {
            seen.push_back("root");
        }
        return false;
    };
    send(router, event::input::Mouse_ButtonPressed{MouseButton::Left, 10, 5});
    SIF_CHECK((seen == std::vector<std::string>{"menu"}));
}

SIF_TEST(router_respects_layers_so_a_modal_blocks_clicks) {
    Ui ui;
    ui::Overlay root;
    // Re-parent the sample UI under an Overlay with a modal on top.
    auto content = std::make_unique<ui::VBox>();
    ui::VBox* column = content.get();
    auto b = std::make_unique<ui::Button>();
    b->width = intrnl::Size::px(100.f);
    b->height = intrnl::Size::px(30.f);
    int clicked = 0;
    b->on_click = [&clicked](ui::Button&) { ++clicked; };
    column->add(std::move(b));
    root.add(std::move(content));
    auto modal = std::make_unique<ui::Rectangle>();
    ui::Rectangle* modal_ptr = modal.get();
    modal->h_align = intrnl::Align::Stretch;
    modal->v_align = intrnl::Align::Stretch;
    modal->layer = rnd::layer::Modal;
    root.add(std::move(modal));
    root.measure({200.f, 200.f});
    root.layout({0.f, 0.f, 200.f, 200.f});

    ui::InputRouter router(root);
    click(router, 10, 10);
    SIF_CHECK(clicked == 0);

    modal_ptr->visible = false;
    click(router, 10, 10);
    SIF_CHECK(clicked == 1);
}

SIF_TEST(router_wheel_scrolls_the_innermost_scroll_view_then_bubbles) {
    ui::Overlay root;
    auto view = std::make_unique<ui::ScrollView>();
    ui::ScrollView* sv = view.get();
    sv->width = intrnl::Size::px(100.f);
    sv->height = intrnl::Size::px(100.f);
    sv->scroll_step = 25.f;
    auto list = std::make_unique<ui::VBox>();
    for (int i = 0; i < 4; ++i) {
        auto r = std::make_unique<ui::Rectangle>();
        r->height = intrnl::Size::px(50.f);
        list->add(std::move(r));
    }
    sv->add(std::move(list));
    root.add(std::move(view));
    root.measure({400.f, 400.f});
    root.layout({0.f, 0.f, 400.f, 400.f});

    int outer_wheels = 0;
    root.event_handler = [&outer_wheels](ui::UIEvent& e) {
        if (e.type == ui::UIEventType::Wheel) {
            ++outer_wheels;
        }
        return false;
    };

    ui::InputRouter router(root);
    SIF_CHECK(send(router, event::input::Mouse_WheelScrolled{-2.f, 50, 50})); // two notches down
    SIF_CHECK(sv->scroll_offset().y == 50.f);
    SIF_CHECK(outer_wheels == 0); // the scroll view used it and stopped it

    send(router, event::input::Mouse_WheelScrolled{-10.f, 50, 50}); // to the end (100)
    SIF_CHECK(sv->scroll_offset().y == 100.f);
    SIF_CHECK(!send(router, event::input::Mouse_WheelScrolled{-1.f, 50, 50})); // at the end...
    SIF_CHECK(outer_wheels == 1);                                               // ...it bubbles on
}

SIF_TEST(router_reset_forgets_state) {
    Ui ui;
    ui::InputRouter router(ui.root);
    send(router, event::input::Mouse_Moved{10, 5});
    router.focus(ui.buttons[0]);
    router.reset();
    SIF_CHECK(!ui.buttons[0]->hovered && !ui.buttons[0]->focused && router.focused() == nullptr);
}

// =================================================================
// Keys (audit 6.1)
// =================================================================

SIF_TEST(original_keys_keep_their_values_and_every_key_has_a_name) {
    // Code that stored or switched on the original fifteen keys must not
    // see them renumbered by the expansion.
    SIF_CHECK(static_cast<int>(Key::Unknown) == 0);
    SIF_CHECK(static_cast<int>(Key::W) == 1);
    SIF_CHECK(static_cast<int>(Key::Num3) == 14);
    SIF_CHECK(static_cast<int>(Key::Backspace) == 15);

    SIF_CHECK(event::input::key_name(Key::X) == "X"); // the X-macro trap: X(X)
    SIF_CHECK(event::input::key_name(Key::F5) == "F5");
    SIF_CHECK(event::input::key_name(Key::PageUp) == "PageUp");
    SIF_CHECK(event::input::key_name(Key::Unknown) == "Unknown");

    std::vector<std::string_view> names;
#define SIF_TEST_KEY_NAME(name) names.push_back(event::input::key_name(Key::name));
    SIF_KEYS(SIF_TEST_KEY_NAME)
#undef SIF_TEST_KEY_NAME
    std::sort(names.begin(), names.end());
    SIF_CHECK(names.size() == 98);
    SIF_CHECK(std::adjacent_find(names.begin(), names.end()) == names.end()); // all distinct
    SIF_CHECK(std::find(names.begin(), names.end(), "Unknown") == names.end()); // every key is named
}

SIF_TEST(tab_moves_focus_into_and_through_the_ui) {
    Ui ui;
    ui::InputRouter router(ui.root);

    SIF_CHECK(send(router, event::input::KeyPressed{Key::Tab, false, false, false, false}));
    SIF_CHECK(router.focused() == ui.buttons[0]); // Tab is the way in
    send(router, event::input::KeyPressed{Key::Tab, false, false, false, false});
    SIF_CHECK(router.focused() == ui.buttons[1]);
    send(router, event::input::KeyPressed{Key::Tab, false, false, true, false}); // Shift+Tab
    SIF_CHECK(router.focused() == ui.buttons[0]);

    router.focus(nullptr);
    send(router, event::input::KeyPressed{Key::Tab, false, false, true, false});
    SIF_CHECK(router.focused() == ui.buttons[3]); // Shift+Tab in from the end

    // Nothing focusable: Tab is left to the game.
    ui::VBox empty;
    ui::InputRouter idle(empty);
    SIF_CHECK(!send(idle, event::input::KeyPressed{Key::Tab, false, false, false, false}));
}
