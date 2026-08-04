/***************************************************************
 * Project:       Render_Engine (SFML backend)
 * File:          ISFML_Event_Source.h
 *
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2025-12-19
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SFML_ISFML_EVENT_SOURCE_H
#define RENDER_ENGINE_SFML_ISFML_EVENT_SOURCE_H

#include "SFML/Window/Event.hpp"

namespace sif::sfml {

    /**
     * @brief Provides SFML window event support within the View.
     *
     * Although a renderer is typically render-only, SFML_Renderer must
     * poll the window to acquire OS-level events such as close, resize
     * and input. These events are tied to the SFML window and cannot be
     * accessed otherwise.
     *
     * This does not violate the separation of concerns: SFML_Renderer
     * only exposes the raw event stream, while interpretation and
     * reaction to events remain the responsibility of the
     * core/application layers.
     */
    class ISFML_Event_Source {
    public:
        virtual ~ISFML_Event_Source() = default;
        virtual bool poll_event(sf::Event& e) = 0;
    };
}

#endif //RENDER_ENGINE_SFML_ISFML_EVENT_SOURCE_H
