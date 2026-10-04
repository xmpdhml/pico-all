#pragma once

#include <stdio.h>

/* ------------------------------------------------------------------ *
 * Debug output switches (compile-time; all default to on)
 *
 *   DEBUG_LOG_ENABLE   Master switch. Controls DEBUG_LOG() as well as the
 *                      ad-hoc debug printf/std::cout lines in src/system.cpp.
 *                      When 0 they expand to nothing: zero code, zero cost,
 *                      arguments are not evaluated.
 *   BOOT_LOG_ENABLE    The [BOOT] bring-up milestones printed via BOOT_LOG()
 *                      through the RTOS-independent direct UART writer.
 *                      Defaults to following the master switch.
 *   FATAL_LOG_ENABLE   Fatal diagnostics only: configASSERT failure and
 *                      "scheduler failed to start". Left ON even in release
 *                      builds by default — they can only ever print on a fatal
 *                      error, so they cost nothing at runtime, and without them
 *                      a fatal startup failure is completely silent (no UART
 *                      output at all, which is very hard to diagnose).
 *
 * To build a quiet release image, pass -DDEBUG_LOG_ENABLE=0 (and, for total
 * silence, -DFATAL_LOG_ENABLE=0) to the compiler; no source edits are needed.
 * Note: the host test build sets DEBUG_LOG_ENABLE=0 via CMake to avoid noise.
 * ------------------------------------------------------------------ */
#ifndef DEBUG_LOG_ENABLE
#define DEBUG_LOG_ENABLE 1
#endif
#ifndef BOOT_LOG_ENABLE
#define BOOT_LOG_ENABLE DEBUG_LOG_ENABLE
#endif
#ifndef FATAL_LOG_ENABLE
#define FATAL_LOG_ENABLE 1
#endif

/* Provided by src/uart_dma_stdio.cpp. Declared here rather than including
 * uart_dma_stdio.h so this header stays usable by the host test build, which
 * compiles against a stub and has no pico-sdk UART headers available. */
#ifdef __cplusplus
extern "C"
#endif
void uart_boot_puts(const char* s);

/* DEBUG_LOG(tag, fmt, ...): print "[tag] msg\n" to stdio.
 * stdio is routed through the UART DMA queue (uart_dma_stdio), so it is safe
 * to call from any task. Needs the scheduler running — the queue is pumped by
 * the uart_tx task — so use BOOT_LOG() before it starts.
 * Example: DEBUG_LOG("SYS", "init done, rows=%d", n); */
#if DEBUG_LOG_ENABLE
    #define DEBUG_LOG(tag, fmt, ...) \
        do { printf("[%s] " fmt "\n", tag, ##__VA_ARGS__); } while (0)
#else
    #define DEBUG_LOG(tag, fmt, ...) ((void)0)
#endif

/* BOOT_LOG(msg): bring-up milestone. Sent through the direct blocking UART
 * writer (no queue, no task), so it still appears before the scheduler starts
 * and when it fails to start. Takes a const char* only. */
#if BOOT_LOG_ENABLE
    #define BOOT_LOG(msg) uart_boot_puts(msg)
#else
    #define BOOT_LOG(msg) ((void)0)
#endif

/* FATAL_LOG(msg): emitted only on a fatal error path. See FATAL_LOG_ENABLE. */
#if FATAL_LOG_ENABLE
    #define FATAL_LOG(msg) uart_boot_puts(msg)
#else
    #define FATAL_LOG(msg) ((void)0)
#endif
