/***************************************************************
 * Project:          Render_Engine
 * File:             Event_Collector.h
 *
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
    class Event_Collector {
    public:
        virtual ~Event_Collector() = default;

        virtual void collect() = 0;
        Event_Store event_store_;
    };
} // namespace sif::event

#endif // RENDER_ENGINE_EVENT_COLLECTOR_H