/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-01
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/internal/Random.h"

#include <algorithm>

namespace sif::intrnl {
    Random & Random::instance()  {
        static Random inst;
        return inst;
    }

    Random::Random() : engine_(std::random_device{}()) {
    }

    void Random::seed(const std::uint32_t value) {
        std::lock_guard lock(mtx_);
        engine_.seed(value);
    }

    void Random::seed_from_entropy() {
        std::lock_guard lock(mtx_);
        engine_.seed(std::random_device{}());
    }

    std::size_t Random::next_index(const std::size_t count) {
        if (count == 0) {
            throw std::invalid_argument("Random::next_index - cannot pick from an empty set");
        }

        // Draw through the widest supported unsigned type rather than
        // std::size_t directly: on platforms where size_t is a typedef
        // for something the distribution does not support, the
        // static_assert in next_int would fire for a perfectly ordinary
        // call.
        using Wide = unsigned long long;
        return static_cast<std::size_t>(
            next_int<Wide>(0, static_cast<Wide>(count) - 1));
    }

    float Random::next_float(const float min, const float max)  {
        if (max < min) {
            throw std::invalid_argument(
                "Random::next_float - empty range [" + std::to_string(min) +
                ", " + std::to_string(max) + ")");
        }

        std::uniform_real_distribution<float> dist(min, max);

        std::lock_guard lock(mtx_);
        return dist(engine_);
    }

    bool Random::chance(const float p)  {
        // Short-circuit the certain outcomes: it makes p <= 0 and p >= 1
        // exact instead of merely very likely, and it costs one branch.
        if (p <= 0.f) {
            return false;
        }
        if (p >= 1.f) {
            return true;
        }

        std::bernoulli_distribution dist(static_cast<double>(p));

        std::lock_guard lock(mtx_);
        return dist(engine_);
    }

    Random::Engine & Random::engine() {
        return engine_;
    }
}
