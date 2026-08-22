/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-11-15
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/internal/Delta_Timer.h"

namespace sif::intrnl {

    Delta_Timer& Delta_Timer::instance() {
        static Delta_Timer inst;
        return inst;
    }

    Delta_Timer::Delta_Timer() : last(clock::now()) {}

    float Delta_Timer::tick() {
        const auto now = clock::now();
        const std::chrono::duration<float> diff = now - last;
        last = now;
        return diff.count(); // seconds
    }
} // namespace sif::intrnl
