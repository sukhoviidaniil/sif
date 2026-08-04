/***************************************************************
* Project:          Render_Engine
* File:             Event_Bus.inl
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-15
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#pragma once

#include <algorithm>

#include "Event.h"
#include "Event_Bus.h"

namespace sif::event{
    template<typename Event>
    Event_Bus::Subscription Event_Bus::subscribe(std::function<void(const Event &)> fn, int priority) {
        auto& list = handlers_[typeid(Event)];
        HandlerId id = next_id_++;

        list.push_back({
            id,
            priority,
            Event::mask,
            [fn = std::move(fn)](const EventConcept& e) {
                fn(static_cast<const EventInstance<Event>&>(e).value);
            }
        });

        // Handlers are kept sorted by descending priority; sort() is the
        // single place that ordering is defined (it used to be sorted
        // twice here, with two different comparators).
        sort(list);

        return { shared_from_this(), typeid(Event), id };
    }

    template<typename Event>
    void Event_Bus::emit(const Event &event) {
        auto it = handlers_.find(typeid(Event));
        if (it == handlers_.end()) {
            return;
        }
        // Handlers take a type-erased EventConcept&, so the concrete
        // event has to be wrapped first (passing &event compiled for
        // nobody: this template was simply never instantiated).
        const EventInstance<Event> instance(event);
        for (const Handler& h : it->second) {
            if (has(h.mask, instance.mask())) {
                h.fn(instance);
            }
        }
    }
}
