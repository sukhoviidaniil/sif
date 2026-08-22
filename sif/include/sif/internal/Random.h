/***************************************************************
 * Project:       render-engine
 * File:          Random.h
 *
 * Author:        Sukhovii Daniil
 * Created:       2025-12-25
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

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace sif::intrnl {

    /**
     * @brief The integer types std::uniform_int_distribution is defined for.
     *
     * [rand.req.genl] permits only these; instantiating the distribution
     * with char, bool or a fixed-width type narrower than short is
     * undefined behaviour that most standard libraries nonetheless
     * compile without a word. next_int() therefore rejects them at
     * compile time instead of producing silent nonsense at run time.
     */
    template<class T>
    inline constexpr bool is_uniform_int_type_v =
        std::is_same_v<T, short> || std::is_same_v<T, int> || std::is_same_v<T, long> || std::is_same_v<T, long long> ||
        std::is_same_v<T, unsigned short> || std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long> ||
        std::is_same_v<T, unsigned long long>;

    /**
     * @brief Singleton random number generator utility.
     *
     * One Mersenne Twister for the whole program, kept as a data member
     * so every draw continues the same sequence instead of restarting a
     * freshly seeded generator per call.
     *
     * @par Range preconditions
     * The standard distributions have preconditions (min <= max for the
     * integer and real distributions, 0 <= p <= 1 for bernoulli) whose
     * violation is undefined behaviour, not an error. Every entry point
     * here either validates and throws, or documents an explicit
     * clamping rule, so a bad argument surfaces at the call site rather
     * than as an impossible index three frames later.
     *
     * @par Thread safety
     * All value-producing members are safe to call from any thread: the
     * engine is guarded by a mutex, which keeps the single global
     * sequence the assignment asks for. The only exception is engine(),
     * which hands out the raw generator - see the warning there, and
     * prefer with_engine().
     *
     * @par Reproducibility
     * seed() makes a run deterministic, which is what allows behaviour
     * that depends on chance (bot decisions, tie-breaking) to be unit
     * tested at all. Production code should leave the default entropy
     * seeding alone.
     */
    class Random {
    public:
        using Engine = std::mt19937; ///< Type of the underlying RNG engine

        /**
         * @brief Returns the singleton instance of Random.
         */
        static Random& instance();

        // ===== Seeding =====

        /**
         * @brief Restarts the sequence from a fixed seed.
         *
         * Intended for tests and for reproducing a reported run; two
         * programs seeded identically produce identical draws.
         */
        void seed(std::uint32_t value);

        /**
         * @brief Reseeds from the system entropy source.
         *
         * Undoes a previous seed() call.
         */
        void seed_from_entropy();

        // ===== Draws =====

        /**
         * @brief Returns a random integer in the closed range [min, max].
         *
         * @throws std::invalid_argument if max < min. That case is
         * almost never a deliberate empty range - it is usually
         * `count - 1` on an empty container, where the unsigned
         * subtraction wrapped - so failing loudly beats returning an
         * index into nothing. Use next_index() for exactly that job.
         */
        template<class Int>
        Int next_int(Int min, Int max) {
            static_assert(is_uniform_int_type_v<Int>, "next_int supports short/int/long/long long and their unsigned "
                                                      "counterparts only; narrower types are undefined behaviour for "
                                                      "std::uniform_int_distribution");

            if (max < min) {
                throw std::invalid_argument("Random::next_int - empty range [" + std::to_string(min) + ", " +
                                            std::to_string(max) + "]");
            }

            std::uniform_int_distribution<Int> dist(min, max);

            std::lock_guard lock(mtx_);
            return dist(engine_);
        }

        /**
         * @brief Returns a random index in [0, count).
         *
         * The safe way to choose one element out of count candidates -
         * "pick a viable direction", "pick a bot personality". Written
         * as its own function because `next_int(0, container.size() - 1)`
         * is the single most common way to get this wrong: on an empty
         * container the unsigned subtraction wraps to SIZE_MAX and the
         * resulting "valid" range yields an out-of-bounds index.
         *
         * @throws std::invalid_argument if count == 0.
         */
        std::size_t next_index(std::size_t count);

        /**
         * @brief Returns a reference to a random element of a non-empty range.
         *
         * @throws std::invalid_argument if the range is empty.
         */
        template<class Range>
        decltype(auto) pick(Range&& range) {
            const auto size = static_cast<std::size_t>(std::distance(std::begin(range), std::end(range)));
            auto it = std::begin(range);
            std::advance(it, static_cast<std::ptrdiff_t>(next_index(size)));
            return *it;
        }

        /**
         * @brief Returns a random float in the half-open range [min, max).
         *
         * Half-open, as std::uniform_real_distribution defines it; the
         * old documentation claimed a closed range.
         *
         * @throws std::invalid_argument if max < min.
         */
        float next_float(float min = 0.f, float max = 1.f);

        /**
         * @brief Returns true with probability p.
         *
         * @param p Probability, clamped to [0, 1]: p <= 0 always yields
         * false and p >= 1 always yields true. Clamped rather than
         * rejected because a probability drifting slightly out of range
         * (an accumulated difficulty modifier, say) has an obvious
         * intended meaning, whereas an empty integer range does not.
         */
        bool chance(float p);

        /**
         * @brief Runs fn with exclusive access to the underlying engine.
         *
         * The supported way to use the shared generator with a standard
         * algorithm, e.g.
         * @code
         * random.with_engine([&](Random::Engine& e) { std::shuffle(v.begin(), v.end(), e); });
         * @endcode
         */
        template<class Fn>
        decltype(auto) with_engine(Fn&& fn) {
            std::lock_guard lock(mtx_);
            return fn(engine_);
        }

        /**
         * @brief Returns a reference to the underlying RNG engine.
         *
         * @warning Unsynchronised, and it lets callers reseed or copy
         * the generator, which defeats the "one sequence" guarantee.
         * Kept for single-threaded interop with standard algorithms;
         * prefer with_engine().
         */
        Engine& engine();

        Random(const Random&) = delete;
        Random& operator=(const Random&) = delete;
        Random(Random&&) = delete;
        Random& operator=(Random&&) = delete;

    private:
        /**
         * @brief Constructs the singleton instance and seeds the engine.
         */
        Random();

        mutable std::mutex mtx_; ///< Guards engine_
        Engine engine_;          ///< The underlying Mersenne Twister engine
    };

    // -----------------------------------------------------------------
    // Convenience wrappers.
    //
    // Functions, not macros: they respect namespaces, are visible to
    // IDEs and debuggers, and cannot be broken by a stray #define. The
    // RAND_* macros below are kept for the existing call sites and now
    // expand to fully qualified names - the previous spelling started
    // at `intrnl::`, so the macros only compiled inside namespace sif
    // and were unusable from any game namespace.
    // -----------------------------------------------------------------

    template<class Int>
    Int rand_int(Int min, Int max) {
        return Random::instance().next_int<Int>(min, max);
    }

    inline std::size_t rand_index(const std::size_t count) {
        return Random::instance().next_index(count);
    }

    inline float rand_float(const float min = 0.f, const float max = 1.f) {
        return Random::instance().next_float(min, max);
    }

    inline bool rand_chance(const float p) {
        return Random::instance().chance(p);
    }
} // namespace sif::intrnl

#define RAND_INT(min, max) ::sif::intrnl::rand_int((min), (max))
#define RAND_INDEX(count) ::sif::intrnl::rand_index((count))
#define RAND_FLOAT(min, max) ::sif::intrnl::rand_float((min), (max))
#define RAND_CHANCE(p) ::sif::intrnl::rand_chance((p))
#define RAND_ENGINE() ::sif::intrnl::Random::instance().engine()

#endif // RENDER_ENGINE_RANDOM_H
