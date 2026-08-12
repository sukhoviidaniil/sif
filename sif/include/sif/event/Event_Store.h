/***************************************************************
* Project:          Render_Engine
* File:             Event_Store.h
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-18
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_EVENT_STORE_H
#define RENDER_ENGINE_EVENT_STORE_H
#include <memory>
#include <vector>

#include "Event.h"

namespace sif::event {
    /**
     * @brief Container for storing events in a type-erased form.
     *
     * Provides first-in-first-out access to events and utilities for
     * type and mask-based inspection.
     */
    class Event_Store {
    public:
        /**
         * @brief Returns the number of stored events.
         */
        [[nodiscard]] std::size_t size() const noexcept;

        /**
         * @brief Checks whether the store is empty.
         */
        [[nodiscard]] bool empty() const noexcept;

        /**
         * @brief Checks whether the event at the given index has a specific type.
         *
         * @param i Index of the event.
         * @param t Expected type.
         */
        [[nodiscard]] bool is(std::size_t i, std::type_index t) const;

        /**
         * @brief Checks whether the event at the given index matches a mask.
         *
         * @param i Index of the event.
         * @param mask Mask to test.
         */
        [[nodiscard]] bool matches(std::size_t i, EventMask mask) const;

        /**
         * @brief Stores a concrete event.
         *
         * @tparam Event Event type.
         * @param e Event value.
         */
        template<typename Event>
        void push(Event e);

        /**
         * @brief Stores a type-erased event by cloning it.
         *
         * @param e Event concept instance.
         */
        void push_concept(EventConcept &e);


        /**
         * @brief Removes and returns the last event as a concrete type.
         *
         * @tparam Event Expected event type.
         *
         * @throws std::out_of_range if the store is empty.
         */
        template<class Event>
        Event pop();

        /**
         * @brief Removes and returns the last event as a type-erased object.
         * @throws std::out_of_range if the store is empty.
         */
        std::unique_ptr<EventConcept> pop_concept();

        /**
         * @brief Returns the last stored event without removing it.
         */
        [[nodiscard]] const EventConcept& peek() const;

        /**
         * @brief Returns the event at a specific index.
         *
         * @param i Index of the event.
         */
        [[nodiscard]] const EventConcept& at(std::size_t i) const;


        /**
         * @brief Accesses a stored event as a concrete type.
         *
         * @tparam Event Expected event type.
         * @param i Index of the event.
         */
        template<typename Event>
        const Event& get(std::size_t i) const;
    private:
        /// Stored events
        std::vector<std::unique_ptr<EventConcept>> events_;
    };
}
#include "Event_Store.inl"

#endif //RENDER_ENGINE_EVENT_STORE_H