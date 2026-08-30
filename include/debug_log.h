#pragma once

#include <stdio.h>

/* Debug logging master switch: 1 = on, 0 = off.
 * When off, all DEBUG_LOG expands to nothing (zero cost, arguments not evaluated).
 * Note: the host test build defines DEBUG_LOG_ENABLE=0 via CMake to avoid noise. */
#ifndef DEBUG_LOG_ENABLE
#define DEBUG_LOG_ENABLE 1
#endif

/* DEBUG_LOG(tag, fmt, ...): print "[tag] msg\n" to stdio.
 * stdio is routed through the UART DMA queue (uart_dma_stdio), so it is safe
 * to call from any task.
 * Example: DEBUG_LOG("SYS", "init done, rows=%d", n); */
#if DEBUG_LOG_ENABLE
    #define DEBUG_LOG(tag, fmt, ...) \
        do { printf("[%s] " fmt "\n", tag, ##__VA_ARGS__); } while (0)
#else
    #define DEBUG_LOG(tag, fmt, ...) ((void)0)
#endif
