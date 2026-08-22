/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-02-01
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/event/Observer.h"

namespace sif::event {
    Observer::Observer() = default;

    Observer::~Observer() {
        un_track_all();
    }

    void Observer::track(Event_Bus::Subscription s) {
        subs_.push_back(std::move(s));
    }

    void Observer::un_track_all() {
        for (auto& sub : subs_) {
            sub.unsubscribe();
        }
        subs_.clear();
    }
} // namespace sif::event
