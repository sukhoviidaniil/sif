/***************************************************************
* Project:          Render_Engine
* File:             window.hpp
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-21
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_EV_WINDOW_HPP
#define RENDER_ENGINE_EV_WINDOW_HPP

#include "sif/event/Event.h"

namespace sif::event::window {
    struct Window_Closed{
        static constexpr EventMask mask = EventMask::Window;
    };

    struct Window_Resized {
        static constexpr EventMask mask = EventMask::Window;

        std::uint32_t width;
        std::uint32_t height;
    };

    struct Window_FocusGained {
        static constexpr EventMask mask = EventMask::Window;
    };

    struct Window_FocusLost {
        static constexpr EventMask mask = EventMask::Window;
    };
}

#endif //RENDER_ENGINE_EV_WINDOW_HPP