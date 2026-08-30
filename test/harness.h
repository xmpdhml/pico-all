#pragma once

#include <cstdio>
#include <vector>
#include <string>

/* Minimal unit-test framework (zero dependencies, compiled directly by host g++).
 *
 * Usage:
 *   #include "harness.h"
 *   UT_CASE(name) { CHECK(...); CHECK_EQ(a, b); }
 */

namespace ut {

struct Case {
    const char* name;
    void (*fn)();
};

inline std::vector<Case>& registry() {
    static std::vector<Case> r;
    return r;
}

struct Registrar {
    Registrar(const char* n, void (*f)()) { registry().push_back({n, f}); }
};

inline int& g_checks() { static int c = 0; return c; }
inline int& g_fails()  { static int f = 0; return f; }

int run_all();   // Implemented in test_main.cpp

} // namespace ut

#define UT_CONCAT_IMPL(a, b) a##b
#define UT_CONCAT(a, b) UT_CONCAT_IMPL(a, b)

#define UT_CASE(name) \
    static void UT_CONCAT(ut_fn_, name)(); \
    static ::ut::Registrar UT_CONCAT(ut_reg_, name)(#name, UT_CONCAT(ut_fn_, name)); \
    static void UT_CONCAT(ut_fn_, name)()

#define CHECK(cond) \
    do { \
        ::ut::g_checks()++; \
        if (!(cond)) { ::ut::g_fails()++; std::printf("    FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } \
    } while (0)

#define CHECK_EQ(a, b) \
    do { \
        ::ut::g_checks()++; \
        auto _va = (a); \
        auto _vb = (b); \
        if (!(_va == _vb)) { \
            ::ut::g_fails()++; \
            std::printf("    FAIL %s:%d  %s == %s  (%lld vs %lld)\n", __FILE__, __LINE__, #a, #b, (long long)_va, (long long)_vb); \
        } \
    } while (0)
