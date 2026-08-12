/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2025-12-19
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "SFML_Event_Collector.h"

#include "sif/event/events/input_Keyboard.h"
#include "sif/event/events/input_Mouse.h"
#include "sif/event/events/window.hpp"

namespace sif::sfml {
    namespace {
        event::input::Key to_key(const sf::Keyboard::Key k) {
            using K = sf::Keyboard::Key;
            using I = event::input::Key;

            switch (k) {
                case K::W:      return I::W;
                case K::A:      return I::A;
                case K::S:      return I::S;
                case K::D:      return I::D;
                case K::Escape: return I::Escape;
                case K::Enter:  return I::Enter;
                case K::Space:  return I::Space;
                case K::Left:   return I::Left;
                case K::Right:  return I::Right;
                case K::Up:     return I::Up;
                case K::Down:   return I::Down;
                case K::Num1:   return I::Num1;
                case K::Num2:   return I::Num2;
                case K::Num3:   return I::Num3;
                case K::Backspace: return I::Backspace;
                default:        return I::Unknown;
            }
        }

        event::input::MouseButton to_button(const sf::Mouse::Button b) {
            using B = sf::Mouse::Button;
            using M = event::input::MouseButton;

            switch (b) {
                case B::Left:     return M::Left;
                case B::Right:    return M::Right;
                case B::Middle:   return M::Middle;
                case B::XButton1: return M::XButton1;
                case B::XButton2: return M::XButton2;
                default:          return M::Left;
            }
        }
    }

    SFML_Event_Collector::SFML_Event_Collector(ISFML_Event_Source &source) : source_(source) {}

    void SFML_Event_Collector::collect()  {
        sf::Event event{};
        while (source_.poll_event(event)) {
            dispatch(event);
        }
    }

    void SFML_Event_Collector::dispatch(const sf::Event &e)  {
        using namespace event;
        switch (e.type) {

            // ---------- Window ----------
            case sf::Event::Closed:
                event_store_.push(window::Window_Closed{});
                break;

            case sf::Event::Resized:
                event_store_.push(window::Window_Resized{
                    e.size.width,
                    e.size.height
                });
                break;

            case sf::Event::GainedFocus:
                event_store_.push(window::Window_FocusGained{});
                break;

            case sf::Event::LostFocus:
                event_store_.push(window::Window_FocusLost{});
                break;

            // ---------- Keyboard ----------
            case sf::Event::KeyPressed:
                event_store_.push(input::KeyPressed{
                    to_key(e.key.code),
                    e.key.alt,
                    e.key.control,
                    e.key.shift,
                    e.key.system
                });
                break;

            case sf::Event::KeyReleased:
                // Previously this reinterpret-cast the raw SFML key code
                // into the engine enum, which silently produced wrong
                // keys for everything past 'D'. Use the same mapping as
                // KeyPressed.
                event_store_.push(input::KeyReleased{
                    to_key(e.key.code)
                });
                break;

            // ---------- Text ----------
            case sf::Event::TextEntered:
                // SFML has already applied the keyboard layout and the
                // modifiers, so this is the character the user meant -
                // whatever their keyboard looks like. Deriving it from a
                // key code here would mean re-implementing that badly.
                event_store_.push(input::TextEntered{
                    static_cast<char32_t>(e.text.unicode)
                });
                break;

            // ---------- Mouse ----------
            case sf::Event::MouseMoved:
                event_store_.push(input::Mouse_Moved{
                    e.mouseMove.x,
                    e.mouseMove.y
                });
                break;

            case sf::Event::MouseButtonPressed:
                event_store_.push(input::Mouse_ButtonPressed{
                    to_button(e.mouseButton.button),
                    e.mouseButton.x,
                    e.mouseButton.y
                });
                break;

            case sf::Event::MouseButtonReleased:
                event_store_.push(input::Mouse_ButtonReleased{
                    to_button(e.mouseButton.button),
                    e.mouseButton.x,
                    e.mouseButton.y
                });
                break;

            case sf::Event::MouseWheelScrolled:
                event_store_.push(input::Mouse_WheelScrolled{
                    e.mouseWheelScroll.delta,
                    e.mouseWheelScroll.x,
                    e.mouseWheelScroll.y
                });
                break;

            default:
                break;
        }
    }
}
