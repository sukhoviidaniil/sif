/***************************************************************
* Project:          Render_Engine
* File:             Events.h
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-17
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_EVENTS_H
#define RENDER_ENGINE_EVENTS_H

#include <cstdint>
#include <typeindex>
#include <utility>
#include <memory>

namespace sif::event {
    /**
     * @brief Bitmask describing event categories.
     *
     * Used to classify events and allow filtering by type of origin
     * (input, window system, game logic, etc.).
     */
    enum class EventMask : std::uint32_t {
        None   = 0,        // 0000
        Input  = 1 << 0,   // 0001
        Window = 1 << 1,   // 0010
        System = 1 << 2,   // 0100
        Program= 1 << 3,   // 1000
    };


    /**
     * @brief Bitwise AND operator for EventMask.
     *
     * Allows combining and testing flags.
     */
    inline EventMask operator&(EventMask a, EventMask b) {
        return static_cast<EventMask>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    /**
     * @brief Bitwise OR operator for EventMask.
     *
     * Allows combining multiple event categories.
     */
    inline EventMask operator|(EventMask a, EventMask b) {
        return static_cast<EventMask>(
            static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
        );
    }

    /**
     * @brief Checks whether a flag is present in a mask.
     *
     * @param value Combined event mask.
     * @param flag  Flag to test.
     * @return true if the flag is set, false otherwise.
     */
    inline bool has(const EventMask value, const EventMask flag) {
        return (value & flag) != EventMask::None;
    }

    /**
     * @brief Type-erased interface for events.
     *
     * Provides a common abstraction for storing and handling
     * events of different concrete types.
     */
    class EventConcept {
    public:
        virtual ~EventConcept() = default;
        /**
         * @brief Returns the event category mask.
         */
        [[nodiscard]] virtual event::EventMask mask() const = 0;

        /**
         * @brief Returns the runtime type information of the event.
         */
        [[nodiscard]] virtual std::type_index type() const = 0;

        /**
         * @brief Returns a pointer to the stored event data.
         *
         */
        [[nodiscard]] virtual const void* data() const = 0;

        /**
         * @brief Creates a deep copy of the event.
         */
        [[nodiscard]] virtual std::unique_ptr<EventConcept> clone() = 0;
    };

    /**
     * @brief Concrete event wrapper for a specific event type.
     *
     * @tparam Event User-defined event type.
     *
     * Wraps a concrete event value and exposes it through
     * the EventConcept interface.
     */
    template<typename Event>
    class EventInstance final : public EventConcept {
    public:
        Event value;  ///< Stored event value

        /**
         * @brief Constructs an event instance.
         *
         * @param v Event value to store.
         */
        explicit EventInstance(Event v) : value(std::move(v)) {}

        /**
         * @brief Returns the event mask defined by the event type.
         */
        [[nodiscard]] EventMask mask() const override {
            return Event::mask;
        }

        /**
         * @brief Returns the runtime type of the stored event.
         */
        [[nodiscard]] std::type_index type() const override {
            return typeid(Event);
        }

        /**
         * @brief Returns a pointer to the stored event value.
         */
        [[nodiscard]] const void* data() const override { return &value; }


        /**
         * @brief Creates a copy of this event instance.
         */
        [[nodiscard]] std::unique_ptr<event::EventConcept> clone() override {
            return std::make_unique<EventInstance<Event>>(value);
        }
    };
}

#endif //RENDER_ENGINE_EVENTS_H