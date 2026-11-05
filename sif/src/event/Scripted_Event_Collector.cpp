/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/event/Scripted_Event_Collector.h"

namespace sif::event {
void Scripted_Event_Collector::collect() {
    while (!pending_.empty()) {
        const std::unique_ptr<EventConcept> e = pending_.pop_concept();
        event_store_.push_concept(*e);
    }
}

std::size_t Scripted_Event_Collector::pending() const { return pending_.size(); }
} // namespace sif::event
