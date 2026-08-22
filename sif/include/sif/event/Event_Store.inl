/***************************************************************
 * Project:          Render_Engine
 * File:             Event_Store.inl
 *
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-18
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#pragma once

#include <stdexcept>

#include "Event_Store.h"

namespace sif::event {

    template<typename Event>
    void Event_Store::push(Event e) {
        static_assert(requires { Event::mask; }, "Event must define static constexpr EventMask mask");
        events_.emplace_back(std::make_unique<EventInstance<Event>>(std::move(e)));
    }

    template<typename Event>
    Event Event_Store::pop() {
        if (events_.empty()) {
            throw std::out_of_range("Event_Store is empty");
        }

        // First in, first out - the same order as pop_concept(). Two pop
        // functions on one container that disagree about which end they
        // work from would be a trap for whoever used the other one.
        auto& e = *events_.front();
        auto* inst = static_cast<EventInstance<Event>*>(&e);

        Event value = std::move(inst->value);
        events_.erase(events_.begin());
        return value;
    }

    template<typename Event>
    const Event& Event_Store::get(std::size_t i) const {
        auto& e = *events_[i];
        // it is assumed that the type has already been verified
        return static_cast<const EventInstance<Event>&>(e).value;
    }
} // namespace sif::event
