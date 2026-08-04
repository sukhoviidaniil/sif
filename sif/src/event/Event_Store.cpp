/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-20
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#include "sif/event/Event_Store.h"

namespace sif::event {
    std::size_t Event_Store::size() const noexcept {
        return events_.size();
    }

    bool Event_Store::empty() const noexcept {
        return events_.empty();
    }

    void Event_Store::push_concept(EventConcept& e) {
        events_.push_back(e.clone());
    }

    std::unique_ptr<EventConcept> Event_Store::pop_concept() {
        if (events_.empty()) {
            // return nullptr;
            throw std::out_of_range("Event_Store is empty");
        }

        auto ptr = std::move(events_.back());
        events_.pop_back();
        return ptr;
    }

    const EventConcept& Event_Store::peek() const {
        return *events_.back();
    }

    const EventConcept& Event_Store::at(std::size_t i) const {
        return *events_[i];
    }

    bool Event_Store::is(const std::size_t i, const std::type_index t) const {
        return events_[i]->type() == t;
    }

    bool Event_Store::matches(const std::size_t i, const EventMask mask) const {
        return static_cast<bool>(events_[i]->mask() & mask);
    }
}