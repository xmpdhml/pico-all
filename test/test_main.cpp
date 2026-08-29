#include "harness.h"

namespace ut {

int run_all() {
    int fails = 0, total = 0;
    for (auto& c : registry()) {
        const int f0 = g_fails(), c0 = g_checks();
        c.fn();
        const int cf = g_fails() - f0;
        const int cc = g_checks() - c0;
        total++;
        if (cf) { std::printf("[FAIL] %s  (%d/%d checks failed)\n", c.name, cf, cc); fails++; }
        else    { std::printf("[ ok ] %s  (%d checks)\n", c.name, cc); }
    }
    std::printf("\n%d case(s), %d failed\n", total, fails);
    return fails ? 1 : 0;
}

} // namespace ut

int main() {
    return ::ut::run_all();
}
