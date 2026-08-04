/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"

#include <iostream>

namespace sif::test {
    namespace {
        // Failure counter for the test case currently executing;
        // reset by run_all() before each case and read right after.
        int g_current_case_failures = 0;
    }

    TestRegistry& TestRegistry::instance() {
        static TestRegistry registry;
        return registry;
    }

    void TestRegistry::add(TestCase tc) {
        cases_.push_back(std::move(tc));
    }

    int TestRegistry::run_all() {
        int total_failures = 0;

        for (const auto& [name, fn] : cases_) {
            g_current_case_failures = 0;
            std::cout << "== " << name << " ==" << std::endl;
            fn();
            if (g_current_case_failures > 0) {
                std::cout << "-- " << name << ": " << g_current_case_failures << " failure(s) --" << std::endl;
            }
            total_failures += g_current_case_failures;
        }

        std::cout << std::endl;
        std::cout << cases_.size() << " test case(s) run, "
                  << total_failures << " assertion failure(s)." << std::endl;
        std::cout << (total_failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << std::endl;

        return total_failures;
    }

    Registrar::Registrar(const std::string& name, std::function<void()> fn) {
        TestRegistry::instance().add(TestCase{name, std::move(fn)});
    }

    void check(const bool condition, const std::string& description) {
        std::cout << "  [" << (condition ? "PASS" : "FAIL") << "] " << description << std::endl;
        if (!condition) {
            ++g_current_case_failures;
        }
    }
}
