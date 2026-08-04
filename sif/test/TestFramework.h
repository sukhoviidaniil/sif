/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_TEST_FRAMEWORK_H
#define RENDER_ENGINE_TEST_FRAMEWORK_H

#include <functional>
#include <string>
#include <vector>

namespace sif::test {
    /**
     * @brief A single named test case.
     */
    struct TestCase {
        std::string name;
        std::function<void()> fn;
    };

    /**
     * @brief Global registry of test cases.
     *
     * Singleton pattern: exactly one registry collects every SIF_TEST
     * defined anywhere in the test sources, regardless of which .cpp
     * file (and therefore which static-initialization order) they are
     * defined in, so new test files never need to be "wired up"
     * anywhere else - this is what makes the suite extensible: adding
     * a test case is just adding a SIF_TEST(...) block to any test
     * .cpp, new or existing.
     */
    class TestRegistry {
    public:
        static TestRegistry& instance();

        void add(TestCase tc);

        /**
         * @brief Runs every registered test case and prints PASS/FAIL
         * for each assertion.
         *
         * @return Number of failed assertions (0 means success).
         */
        int run_all();

    private:
        TestRegistry() = default;
        std::vector<TestCase> cases_;
    };

    /**
     * @brief Registers a test case at static-initialization time.
     *
     * Constructed once per SIF_TEST(...) invocation (see the macro
     * below); its only job is to add the test case to TestRegistry.
     */
    struct Registrar {
        Registrar(const std::string& name, std::function<void()> fn);
    };

    /**
     * @brief Records a single pass/fail check within the currently
     * running test case.
     *
     * @param condition Result of the check.
     * @param description Human-readable description of what was checked.
     */
    void check(bool condition, const std::string& description);
}

/**
 * @brief Defines a self-registering test case.
 *
 * Usage:
 *   SIF_TEST(my_feature_does_the_thing) {
 *       SIF_CHECK(some_condition);
 *   }
 *
 * No central list of tests to update: the test case registers itself
 * the first time any translation unit containing it is linked in, so
 * covering a new scenario is just adding a new SIF_TEST block, in this
 * file or a new one.
 */
#define SIF_TEST(name) \
    static void sif_test_fn_##name(); \
    static ::sif::test::Registrar sif_test_reg_##name(#name, sif_test_fn_##name); \
    static void sif_test_fn_##name()

/**
 * @brief Checks a boolean condition within a SIF_TEST body.
 *
 * Records both the result and the source expression (stringified) so
 * failures are traceable without a debugger.
 */
#define SIF_CHECK(cond) ::sif::test::check((cond), #cond)

#endif //RENDER_ENGINE_TEST_FRAMEWORK_H
