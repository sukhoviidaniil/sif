/***************************************************************
 * Project:       render-engine
 * File:          Random.h
 *
 * Author:        Sukhovii Daniil
 * Created:       2025-12-25
 * Modified:      []
 *
 * Description:   []
 *
 * Contact:
 *   Email:       sukhovii.daniil@gmail.com
 *
 * Disclaimer:
 *   This file is part of render-engine.
 *   Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_RANDOM_H
#define RENDER_ENGINE_RANDOM_H


#define RAND_INT(min, max) \
intrnl::Random::instance().next_int((min), (max))

#define RAND_FLOAT(min, max) \
intrnl::Random::instance().next_float((min), (max))

#define RAND_CHANCE(p) \
intrnl::Random::instance().chance((p))

#define RAND_ENGINE() \
intrnl::Random::instance().engine()

#include <random>

namespace sif::intrnl{

    /**
     * @brief Singleton random number generator utility.
     *
     * Provides methods for generating random integers, floats,
     * probability-based outcomes, and access to the underlying engine.
     */
    class Random {
    public:
        using Engine = std::mt19937; ///< Type of the underlying RNG engine

        /**
         * @brief Returns the singleton instance of Random.
         */
        static Random& instance();

        /**
         * @brief Returns a random integer in the range [min, max].
         *
         * @param min Minimum value (inclusive)
         * @param max Maximum value (inclusive)
         */
        template<class Int>
        Int next_int(Int min, Int max) {
            std::uniform_int_distribution<Int> dist(min, max);
            return dist(engine_);
        }

        /**
         * @brief Returns a random float in the range [min, max].
         *
         * @param min Minimum value (default 0.f)
         * @param max Maximum value (default 1.f)
         */
        float next_float(float min = 0.f, float max = 1.f);

        /**
         * @brief Returns true with probability p.
         *
         * @param p Probability (0.f to 1.f)
         */
        bool chance(float p);

        /**
         * @brief Returns a reference to the underlying RNG engine.
         */
        Engine& engine();

        Random(const Random&) = delete;
        Random& operator=(const Random&) = delete;

    private:
        /**
         * @brief Constructs the singleton instance and seeds the engine.
         */
        Random();

        Engine engine_; ///< The underlying Mersenne Twister engine
    };
}

#endif //RENDER_ENGINE_RANDOM_H