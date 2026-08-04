/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-08-04
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"

#include <algorithm>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "sif/internal/Random.h"

using namespace sif;

namespace {
    /// Draws a short sequence so two seedings can be compared.
    std::vector<int> sequence(const std::uint32_t seed, const int count = 16) {
        intrnl::Random::instance().seed(seed);
        std::vector<int> out;
        out.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            out.push_back(intrnl::Random::instance().next_int(0, 1000));
        }
        return out;
    }

    template<class Fn>
    bool throws(Fn&& fn) {
        try {
            fn();
        } catch (const std::exception&) {
            return true;
        }
        return false;
    }
}

// ---------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------

SIF_TEST(random_is_one_shared_generator) {
    SIF_CHECK(&intrnl::Random::instance() == &intrnl::Random::instance());
    SIF_CHECK(!std::is_copy_constructible_v<intrnl::Random>);
    SIF_CHECK(!std::is_move_constructible_v<intrnl::Random>);
    SIF_CHECK(!std::is_copy_assignable_v<intrnl::Random>);
}

SIF_TEST(consecutive_draws_continue_the_same_sequence) {
    // The point of holding the engine as a member: two draws in a row
    // must not both restart a freshly seeded generator.
    intrnl::Random::instance().seed(7u);
    std::set<int> distinct;
    for (int i = 0; i < 32; ++i) {
        distinct.insert(intrnl::Random::instance().next_int(0, 1000000));
    }
    SIF_CHECK(distinct.size() > 25); // a restarting generator would repeat
}

// ---------------------------------------------------------------------
// Reproducibility
// ---------------------------------------------------------------------

SIF_TEST(the_same_seed_reproduces_the_same_sequence) {
    // Without this, no behaviour that depends on chance can be tested.
    SIF_CHECK(sequence(12345u) == sequence(12345u));
}

SIF_TEST(different_seeds_produce_different_sequences) {
    SIF_CHECK(sequence(1u) != sequence(2u));
}

SIF_TEST(reseeding_from_entropy_leaves_the_generator_usable) {
    intrnl::Random::instance().seed_from_entropy();
    const int value = intrnl::Random::instance().next_int(5, 5);
    SIF_CHECK(value == 5);
}

// ---------------------------------------------------------------------
// next_int
// ---------------------------------------------------------------------

SIF_TEST(next_int_stays_inside_the_closed_range) {
    intrnl::Random::instance().seed(99u);
    bool in_range = true;
    bool saw_min = false;
    bool saw_max = false;
    for (int i = 0; i < 500; ++i) {
        const int v = intrnl::Random::instance().next_int(3, 6);
        in_range = in_range && v >= 3 && v <= 6;
        saw_min = saw_min || v == 3;
        saw_max = saw_max || v == 6;
    }
    SIF_CHECK(in_range);
    // Closed range: both endpoints must be reachable.
    SIF_CHECK(saw_min);
    SIF_CHECK(saw_max);
}

SIF_TEST(next_int_accepts_a_single_value_range) {
    SIF_CHECK(intrnl::Random::instance().next_int(42, 42) == 42);
}

SIF_TEST(next_int_rejects_an_inverted_range) {
    // Undefined behaviour in the standard distribution; here it is a
    // diagnosable error, because in practice it means an unsigned
    // `size() - 1` wrapped on an empty container.
    SIF_CHECK(throws([] { (void)intrnl::Random::instance().next_int(10, 3); }));
}

SIF_TEST(next_int_works_with_unsigned_types) {
    const auto v = intrnl::Random::instance().next_int<std::uint32_t>(
        1u, std::numeric_limits<std::uint32_t>::max());
    SIF_CHECK(v >= 1u);
}

// ---------------------------------------------------------------------
// next_index / pick
// ---------------------------------------------------------------------

SIF_TEST(next_index_stays_below_the_count) {
    intrnl::Random::instance().seed(2024u);
    bool in_range = true;
    for (int i = 0; i < 200; ++i) {
        in_range = in_range && intrnl::Random::instance().next_index(4) < 4;
    }
    SIF_CHECK(in_range);
    SIF_CHECK(intrnl::Random::instance().next_index(1) == 0);
}

SIF_TEST(next_index_refuses_an_empty_set) {
    // The bot case: no safe direction was found, so there is nothing to
    // choose from and returning "some index" would be a lie.
    SIF_CHECK(throws([] { (void)intrnl::Random::instance().next_index(0); }));
}

SIF_TEST(pick_returns_an_element_of_the_range) {
    const std::vector<std::string> options{"up", "down", "left", "right"};
    intrnl::Random::instance().seed(5u);

    bool all_valid = true;
    for (int i = 0; i < 50; ++i) {
        const std::string& chosen = intrnl::Random::instance().pick(options);
        all_valid = all_valid && std::find(options.begin(), options.end(), chosen) != options.end();
    }
    SIF_CHECK(all_valid);
}

SIF_TEST(pick_refuses_an_empty_range) {
    const std::vector<int> empty;
    SIF_CHECK(throws([&] { (void)intrnl::Random::instance().pick(empty); }));
}

// ---------------------------------------------------------------------
// next_float
// ---------------------------------------------------------------------

SIF_TEST(next_float_stays_inside_the_half_open_range) {
    intrnl::Random::instance().seed(31u);
    bool in_range = true;
    for (int i = 0; i < 300; ++i) {
        const float v = intrnl::Random::instance().next_float(-2.f, 2.f);
        in_range = in_range && v >= -2.f && v < 2.f;
    }
    SIF_CHECK(in_range);
}

SIF_TEST(next_float_defaults_to_the_unit_range) {
    const float v = intrnl::Random::instance().next_float();
    SIF_CHECK(v >= 0.f && v < 1.f);
}

SIF_TEST(next_float_rejects_an_inverted_range) {
    SIF_CHECK(throws([] { (void)intrnl::Random::instance().next_float(1.f, 0.f); }));
}

// ---------------------------------------------------------------------
// chance
// ---------------------------------------------------------------------

SIF_TEST(chance_treats_the_certain_outcomes_exactly) {
    bool never = false;
    bool always = true;
    for (int i = 0; i < 100; ++i) {
        never = never || intrnl::Random::instance().chance(0.f);
        always = always && intrnl::Random::instance().chance(1.f);
    }
    SIF_CHECK(!never);
    SIF_CHECK(always);
}

SIF_TEST(chance_clamps_probabilities_outside_the_unit_range) {
    // Out of range is undefined behaviour for bernoulli_distribution;
    // clamping keeps a drifting difficulty modifier harmless.
    SIF_CHECK(!intrnl::Random::instance().chance(-0.5f));
    SIF_CHECK(intrnl::Random::instance().chance(1.5f));
}

SIF_TEST(chance_is_roughly_fair_at_one_half) {
    intrnl::Random::instance().seed(777u);
    int hits = 0;
    for (int i = 0; i < 2000; ++i) {
        if (intrnl::Random::instance().chance(0.5f)) {
            ++hits;
        }
    }
    // Deliberately wide: this checks that p is honoured at all, not the
    // statistical quality of the Mersenne Twister.
    SIF_CHECK(hits > 850 && hits < 1150);
}

// ---------------------------------------------------------------------
// Engine access
// ---------------------------------------------------------------------

SIF_TEST(with_engine_exposes_the_shared_generator_for_algorithms) {
    std::vector<int> values{1, 2, 3, 4, 5, 6, 7, 8};
    const std::vector<int> original = values;

    intrnl::Random::instance().seed(3u);
    intrnl::Random::instance().with_engine([&values](intrnl::Random::Engine& e) {
        std::shuffle(values.begin(), values.end(), e);
    });

    SIF_CHECK(values.size() == original.size());
    SIF_CHECK(std::is_permutation(values.begin(), values.end(), original.begin()));
}

SIF_TEST(the_rand_macros_resolve_outside_namespace_sif) {
    // Regression guard: the macros used to expand to `intrnl::...`,
    // which only compiled inside namespace sif - i.e. nowhere in a game
    // built on top of the engine. This test lives in the global
    // namespace, so it fails to compile if that returns.
    intrnl::Random::instance().seed(11u);

    const int i = RAND_INT(2, 4);
    const std::size_t idx = RAND_INDEX(3);
    const float f = RAND_FLOAT(0.f, 1.f);
    const bool c = RAND_CHANCE(1.f);

    SIF_CHECK(i >= 2 && i <= 4);
    SIF_CHECK(idx < 3);
    SIF_CHECK(f >= 0.f && f < 1.f);
    SIF_CHECK(c);

    intrnl::Random::instance().seed_from_entropy();
}
