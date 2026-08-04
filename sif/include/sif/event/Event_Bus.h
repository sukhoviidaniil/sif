/***************************************************************
* Project:          Render_Engine
* File:             Event_Bus.h
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-14
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_EVENT_BUS_H
#define RENDER_ENGINE_EVENT_BUS_H

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "Event.h"

namespace sif::event {

    /**
     * @brief Central event dispatching system.
     *
     * Manages event handlers, allows subscribing to specific event types
     * and emitting events to all registered listeners.
     */
    class Event_Bus : public std::enable_shared_from_this<Event_Bus>{
    public:
        /// Unique identifier of a registered handler
        using HandlerId = std::uint64_t;

        // ---------- Subscription handle ----------

        /**
         * @brief RAII handle for an event subscription.
         *
         * Automatically unregisters the handler on destruction.
         */
        class Subscription {
        public:
            /**
             * @brief Creates an empty subscription.
             */
            Subscription() = default;

            /**
             * @brief Creates a subscription bound to an event bus.
             *
             * @param bus  Event bus instance.
             * @param type Event type this subscription listens to.
             * @param id   Handler identifier.
             */
            Subscription(std::shared_ptr<Event_Bus> bus,
                         std::type_index type,
                         HandlerId id);

            /**
             * @brief Unsubscribes the handler from the event bus.
             */
            void unsubscribe();

            /**
             * @brief Destructor.
             *
             * Automatically unsubscribes if still active.
             */
            ~Subscription();

            Subscription(const Subscription&) = delete;
            Subscription& operator=(const Subscription&) = delete;

            /**
             * @brief Move constructor.
             */
            Subscription(Subscription&& other) noexcept;

            /**
             * @brief Move assignment operator.
             */
            Subscription& operator=(Subscription&& other) noexcept;

        private:
            std::shared_ptr<Event_Bus> bus_ = nullptr; ///< Owning event bus
            std::type_index type_{typeid(void)}; ///< Subscribed event type
            HandlerId id_ = 0; ///< Handler identifier
        };

        // ---------- Subscribe ----------

        /**
         * @brief Subscribes a handler to a specific event type.
         *
         * @tparam Event Event type to listen for.
         * @param fn       Callback invoked when the event is emitted.
         * @param priority Execution priority of the handler.
         * @return Subscription handle.
         */
        template<typename Event>
        Subscription subscribe(std::function<void(const Event&)> fn, int priority = 0);

        // ---------- Emit ----------

        /**
         * @brief Emits a concrete event instance.
         *
         * @tparam Event Event type.
         * @param event Event value.
         */
        template<typename Event>
        void emit(const Event& event);

        /**
         * @brief Emits a type-erased event.
         *
         * @param e Event concept instance.
         */
        void emit(const EventConcept& e);

    private:
        /**
         * @brief Internal representation of a registered handler.
         */
        struct Handler {
            HandlerId id; ///< Handler identifier
            int priority; ///< Execution priority
            EventMask mask; ///< Event mask filter
            std::function<void(const EventConcept&)> fn; ///< Handler callback
        };

        /**
         * @brief Removes a handler by type and identifier.
         *
         * @param type Event type.
         * @param id   Handler identifier.
         */
        void unsubscribe(std::type_index type, HandlerId id);

        /**
         * @brief Sorts handlers by priority.
         *
         */
        static void sort(std::vector<Handler>& list);

        std::unordered_map<std::type_index, std::vector<Handler>> handlers_; ///< Registered handlers
        HandlerId next_id_ = 1; ///< Next handler id
    };
}

#include "Event_Bus.inl"

#endif //RENDER_ENGINE_EVENT_BUS_H