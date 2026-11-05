/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_SCRIPTED_EVENT_COLLECTOR_H
#define RENDER_ENGINE_SCRIPTED_EVENT_COLLECTOR_H

#include "sif/event/Event_Collector.h"

namespace sif::event {
/**
 * @brief An Event_Collector fed from code instead of a window.
 *
 * The input leg of the headless backend: a test or a CI run queues
 * engine events (a key press, a click) with queue(), and the next
 * collect() delivers everything queued so far, in order - exactly what
 * a platform collector does with the events the OS reported since the
 * previous frame. Lets input-handling code run without a display.
 */
class Scripted_Event_Collector final : public Event_Collector {
public:
    /// @brief Queues an event for the next collect().
    template <typename Event>
    void queue(Event e) {
        pending_.push(std::move(e));
    }

    /// @brief Moves every queued event into event_store_, oldest first.
    void collect() override;

    /// @brief Events queued but not yet collected.
    [[nodiscard]] std::size_t pending() const;

private:
    Event_Store pending_;
};
} // namespace sif::event

#endif // RENDER_ENGINE_SCRIPTED_EVENT_COLLECTOR_H
