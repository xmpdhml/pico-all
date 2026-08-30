#pragma once

// --- RP2350 VARIANT ---
#define PICO_RP2350B 1

// --- UART ---
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 0
#endif
#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 0
#endif
#ifndef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 1
#endif
#ifndef PICO_DEFAULT_UART_BAUD_RATE
#define PICO_DEFAULT_UART_BAUD_RATE 1500000
#endif

/* ------------------- Memory ------------------- */
// 16MB QSPI flash (XiP capable).
// pico_board_cmake_set_default makes CMake use a 16MB FLASH region when
// generating the linker script; the #ifndef/#define lets C/C++ code read
// PICO_FLASH_SIZE_BYTES.
pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (16 * 1024 * 1024))
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

// 8MB PSRAM: CS trace defaults to GP47 (cuttable).
// The firmware does not enable hardware_psram yet; this is only a pin-usage
// placeholder (do not use GP47 as a regular GPIO). To enable it, add
// target_link_libraries(... hardware_psram ...) + pico_enable_psram(... 1).
#ifndef PICO_PSRAM_CS_PIN
#define PICO_PSRAM_CS_PIN 47
#endif

// pico_cmake_set_default PICO_RP2350_A2_SUPPORTED = 1
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif
