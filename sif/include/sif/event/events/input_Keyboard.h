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
        W, A, S, D,
        Escape,
        Enter,
        Space,
        Left, Right, Up, Down,
        Num1, Num2, Num3,
    };

    struct KeyPressed {
        static constexpr EventMask mask = EventMask::Input;
        Key key;
        bool alt;
        bool control;
        bool shift;
        bool system;
    };

    struct KeyReleased {
        static constexpr EventMask mask = EventMask::Input;
        Key key;
    };
}

#endif //RENDER_ENGINE_INPUT_H