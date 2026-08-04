/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-01
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "../../include/sif/internal/Random.h"

namespace sif::intrnl {
    Random & Random::instance()  {
        static Random inst;
        return inst;
    }

    float Random::next_float(float min, float max)  {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(engine_);
    }

    bool Random::chance(float p)  {
        std::bernoulli_distribution dist(p);
        return dist(engine_);
    }

    Random::Engine & Random::engine() {
        return engine_;
    }

    Random::Random() : engine_(
        std::random_device{}()
        ) {

    }
}
