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

/* ------------------- 内存 ------------------- */
// 16MB QSPI 闪存（支持 XiP）。
// pico_board_cmake_set_default 让 CMake 生成链接脚本时 FLASH 区域用 16MB；
// #ifndef/#define 供 C/C++ 代码读取 PICO_FLASH_SIZE_BYTES。
pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (16 * 1024 * 1024))
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

// 8MB PSRAM：CS 走线默认接 GP47（可切割）。
// 当前固件未启用 hardware_psram，仅作引脚占用占位（勿把 GP47 当普通 GPIO 用）；
// 启用需：target_link_libraries(... hardware_psram ...) + pico_enable_psram(... 1)
#ifndef PICO_PSRAM_CS_PIN
#define PICO_PSRAM_CS_PIN 47
#endif

// pico_cmake_set_default PICO_RP2350_A2_SUPPORTED = 1
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif
