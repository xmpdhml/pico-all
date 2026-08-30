#pragma once

/* Unified configuration — every hardware-adjustable definition lives here.
 *
 *  1. Pins: UART / LED / ENCODER / matrix rows & columns
 *  2. Keymap: matrix dimensions + keymap + matrix config
 *
 * Edit only this file for wiring / keymap / encoder changes.
 * Keycode definitions are in key_codes.h.
 */

#include <cstdint>

#include "key_codes.h"
#include "key_matrix.h"

namespace keyboard_config {

/* ============ 1. Pins ============ */

/* Debug UART (IO, forwards printf/DEBUG_LOG through UartDMAStdio) */
inline constexpr uint8_t kUartTxPin = 0;   // GP0 = UART TX
inline constexpr uint8_t kUartRxPin = 1;   // GP1 = UART RX

/* Status LED (heartbeat) */
inline constexpr uint8_t kLedPin = 2;      // GP2

/* Rotary encoder (reserved, not yet wired; adjust to actual wiring) */
inline constexpr uint8_t kEncoderAPin     = 29;   // A phase
inline constexpr uint8_t kEncoderBPin     = 30;   // B phase
inline constexpr uint8_t kEncoderButtonPin = 31;  // Button

/* Key matrix */
inline constexpr uint8_t kRows = 3;
inline constexpr uint8_t kCols = 3;

// Row pins (output, strobe line, driven low when active)
inline constexpr uint8_t kRowPins[kRows] = { 32, 34, 36 };

// Column pins (input, pull-up; a pressed key reads 0)
inline constexpr uint8_t kColPins[kCols] = { 38, 40, 42 };

/* ============ 2. Keymap ============ */

// Keymap: kKeymap[row][col] -> KeyCodes
// Currently a 3×3 placeholder (KEY_A..I).
inline constexpr KeyCodes kKeymap[kRows][kCols] = {
    { KEY_A, KEY_B, KEY_C },
    { KEY_D, KEY_E, KEY_F },
    { KEY_G, KEY_H, KEY_I },
};

// Assembled matrix config (referenced directly by System's constructor)
inline const KeyMatrix::Config kMatrixConfig = {
    .rows       = kRows,
    .cols       = kCols,
    .row_pins   = kRowPins,
    .col_pins   = kColPins,
    .keymap     = &kKeymap[0][0],
    .direction  = KeyMatrix::Direction::RowToCol,
};

} // namespace keyboard_config
