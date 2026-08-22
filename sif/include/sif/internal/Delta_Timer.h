/***************************************************************
 * Project:       render-engine
 * File:          Delta_Timer.h
 *
 * Author:        Sukhovii Daniil
 * Created:       2025-11-15
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
#ifndef RENDER_ENGINE_DELTATIMER_H
#define RENDER_ENGINE_DELTATIMER_H

#include <chrono>

namespace sif::intrnl {

    /**
     * @brief Utility class to measure elapsed time between updates.
     *
     * Uses a steady clock to compute the delta time since the last tick.
     */
    class Delta_Timer {

    public:
        /**
         * @brief Returns the single Delta_Timer of the program.
         *
         * Singleton pattern (Meyers form: a function-local static, so
         * construction is lazy and thread-safe). One instance is the
         * point - every subsystem that scales something by deltaTime
         * must be scaling it by the *same* deltaTime, otherwise the
         * animation clock, the UI transitions and the game logic drift
         * apart within a single frame.
         */
        static Delta_Timer& instance();

        // A singleton that can be copied is not a singleton: without
        // these, `auto t = Delta_Timer::instance();` silently compiles
        // and hands out a second, independently-ticking clock.
        Delta_Timer(const Delta_Timer&) = delete;
        Delta_Timer& operator=(const Delta_Timer&) = delete;
        Delta_Timer(Delta_Timer&&) = delete;
        Delta_Timer& operator=(Delta_Timer&&) = delete;

        /**
         * @brief Returns the time elapsed (in seconds) since the last tick
         *        and updates the internal time point.
         *
         * @return Delta time in seconds.
         */
        float tick();

    private:
        /**
         * @brief Constructs the Delta_Timer and initializes the last time point.
         */
        Delta_Timer();

        using clock = std::chrono::steady_clock;
        clock::time_point last{}; ///< Last recorded time point
    };
} // namespace sif::intrnl

#endif // RENDER_ENGINE_DELTATIMER_H