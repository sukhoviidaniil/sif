/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2025-12-19
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SFML_EVENT_COLLECTOR_H
#define RENDER_ENGINE_SFML_EVENT_COLLECTOR_H

#include "ISFML_Event_Source.h"

#include "sif/event/Event_Collector.h"

namespace sif::sfml {
    /**
     * @brief Translates sf::Event into the engine's own event types.
     *
     * This is the only place in the program that knows what an
     * sf::Event looks like; everything downstream sees engine events,
     * which is what allows the engine to be compiled and tested with
     * no SFML present.
     */
    class SFML_Event_Collector final : public event::Event_Collector {
    public:
        explicit SFML_Event_Collector(ISFML_Event_Source& source);

        void collect() override;

    private:
        void dispatch(const sf::Event& e);

        ISFML_Event_Source& source_;
    };
}

#endif //RENDER_ENGINE_SFML_EVENT_COLLECTOR_H
