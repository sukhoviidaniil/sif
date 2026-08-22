/***************************************************************
 * Project:          Render_Engine
 * File:             Observer.h
 *
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-15
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_OBSERVER_H
#define RENDER_ENGINE_OBSERVER_H

#include "Event_Bus.h"
namespace sif::event {
    /**
     * @brief Observer pattern: Observer is the base "subscriber" role,
     * Event_Bus is the "subject" being observed. Deriving from Observer
     * (rather than manually calling Event_Bus::subscribe/unsubscribe
     * everywhere) guarantees every subscription an object holds is
     * torn down automatically when that object is destroyed, so a
     * dangling callback into a destroyed object is not possible.
     *
     * Base class for objects observing events from an Event_Bus.
     * Manages the lifetime of event subscriptions and ensures that
     * all tracked subscriptions are properly unsubscribed on destruction.
     */
    class Observer {
    public:
        /**
         * @brief Default constructor.
         */
        Observer();

        /**
         * @brief Virtual destructor.
         *
         * Automatically unsubscribes from all tracked events.
         */
        virtual ~Observer();

        // Copying is prohibited.
        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;

        // Allow movement
        Observer(Observer&&) noexcept = default;
        Observer& operator=(Observer&&) noexcept = default;

    protected:
        /**
         * @brief Tracks an event subscription.
         *
         * The subscription will be automatically unsubscribed
         * when the observer is destroyed or untracked.
         *
         * @param s Subscription to track.
         */
        void track(Event_Bus::Subscription s);

        /**
         * @brief Unsubscribes from all tracked subscriptions.
         *
         */
        void un_track_all();

    private:
        /// List of tracked subscriptions
        std::vector<Event_Bus::Subscription> subs_;
    };
} // namespace sif::event

#endif // RENDER_ENGINE_OBSERVER_H