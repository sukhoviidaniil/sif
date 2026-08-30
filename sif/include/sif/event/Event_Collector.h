/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-19
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_EVENT_COLLECTOR_H
#define RENDER_ENGINE_EVENT_COLLECTOR_H

#include "Event_Store.h"

namespace sif::event {
    /**
     * @brief The input half of a graphics backend - turns whatever the
     * platform reports into the engine's own events, once per frame.
     *
     * Renderer draws, AudioPlayer plays, Event_Collector listens - the third
     * leg of the interface a backend has to provide, and produced the same
     * way as the other two: through the Abstract Factory
     * (Graphics_Factory::make_Event_Collector), so the frame loop holds only
     * an `Event_Collector&` and never learns which concrete backend it is
     * talking to.
     *
     * @par The one method
     * collect() is called exactly once per frame, before anything else that
     * frame does. Its job is narrow and total: ask the platform what
     * happened since the last call, translate every one of those platform
     * -specific occurrences into the engine's own event types, and push
     * each onto event_store_. Nothing about *interpreting* an event belongs
     * here - a collector produces raw occurrences, it does not decide what
     * they mean.
     *
     * @par Why event_store_ is a public member, not a return value or a
     * getter
     * There is exactly one caller, once per frame, and it drains the queue
     * completely before the next collect() - App::run()'s own loop pops
     * `event_store_` one event at a time right after calling collect(). A
     * getter here would wrap direct access to the same object with no
     * actual encapsulation gained; the member is public because the class's
     * entire purpose *is* to populate it.
     *
     * @note SFML_Event_Collector (backends/sfml/) is the concrete
     * implementation this program actually runs - see its own comment for
     * why it is "the only place in the program that knows what an sf::Event
     * looks like." This interface is what makes that containment possible:
     * anything past App::run()'s loop deals only in engine events, never in
     * sf::Event, regardless of which backend produced them.
     */
    class Event_Collector {
    public:
        virtual ~Event_Collector() = default;

        virtual void collect() = 0;
        Event_Store event_store_;
    };
} // namespace sif::event

#endif // RENDER_ENGINE_EVENT_COLLECTOR_H