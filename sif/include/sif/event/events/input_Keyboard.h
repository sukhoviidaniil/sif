/***************************************************************
 * Project:          Render_Engine
 * File:             input_Keyboard.h
 *
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-21
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_INPUT_H
#define RENDER_ENGINE_INPUT_H
#include <cstdint>

#include "sif/event/Event.h"

namespace sif::event::input {
    enum class Key : std::uint16_t {
        Unknown,
        W,
        A,
        S,
        D,
        Escape,
        Enter,
        Space,
        Left,
        Right,
        Up,
        Down,
        Num1,
        Num2,
        Num3,
        Backspace,
    };

    struct KeyPressed {
        static constexpr EventMask mask = EventMask::Input;
        Key key;
        bool alt;
        bool control;
        bool shift;
        bool system;
    };

    /**
     * @brief A character the user typed.
     *
     * Separate from KeyPressed on purpose. A key is a *position* on the
     * keyboard; a character is what the OS produced from it after the
     * layout, the modifiers and any compose sequence. Deriving one from
     * the other in the engine would mean re-implementing the platform's
     * keyboard layout handling - and getting it wrong for every layout
     * that is not the developer's own.
     *
     * Backspace and Enter arrive as KeyPressed, not here: they are
     * editing commands rather than text.
     */
    struct TextEntered {
        static constexpr EventMask mask = EventMask::Input;
        char32_t unicode = 0;

        /// @brief True for characters that make sense in a text field.
        [[nodiscard]] bool printable() const { return unicode >= 32 && unicode != 127; }
    };

    struct KeyReleased {
        static constexpr EventMask mask = EventMask::Input;
        Key key;
    };
} // namespace sif::event::input

#endif // RENDER_ENGINE_INPUT_H