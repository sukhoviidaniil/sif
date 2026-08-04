/***************************************************************
* Project:          Render_Engine
* File:             input_Mouse.h
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-21
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_INPUT_MOUSE_H
#define RENDER_ENGINE_INPUT_MOUSE_H
#include <cstdint>

#include "sif/event/Event_Bus.h"

namespace sif::event::input {
    enum class MouseButton : std::uint8_t {
        Left,
        Right,
        Middle,
        XButton1,
        XButton2
    };

    struct Mouse_Moved {
        static constexpr EventMask mask = EventMask::Input;

        int x;
        int y;
    };

    struct Mouse_ButtonPressed {
        static constexpr EventMask mask = EventMask::Input;

        MouseButton button;
        int x;
        int y;
    };

    struct Mouse_ButtonReleased {
        static constexpr EventMask mask = EventMask::Input;

        MouseButton button;
        int x;
        int y;
    };

    struct Mouse_WheelScrolled {
        static constexpr EventMask mask = EventMask::Input;

        float delta;
        int x;
        int y;
    };
}

#endif //RENDER_ENGINE_INPUT_MOUSE_H