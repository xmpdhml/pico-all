/* main.cpp
 * Entry: sys.init() -> sys.run().
 * sys.init() does one-time hardware init; sys.run() creates the FreeRTOS tasks
 * and starts the dual-core SMP scheduler (never returns). */

#include "system.h"
#include "debug_log.h"

int main()
{
    // Note: before sys.init() the UART stdio is not yet enabled, so any
    // DEBUG_LOG output would be silently discarded — hence the first log sits
    // after init().
    sys.init();   // One-time init: UART stdio / USB HID / matrix / LED
    DEBUG_LOG("MAIN", "boot");
    DEBUG_LOG("MAIN", "init done, entering scheduler");
    sys.run();    // Create tasks + start the FreeRTOS SMP scheduler (never returns)
    return 0;
}
