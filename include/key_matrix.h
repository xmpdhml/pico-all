#pragma once

#include <cstdint>

#include "key_codes.h"

/* KeyMatrix: static description of the key matrix (rows/cols, pins, keymap, direction).
 *
 * Describes only the "wiring", touches no hardware; GPIO scanning and debounce
 * are done by KeyScanner. Data is injected at construction and read-only at
 * runtime; the concrete layout (keymap) is defined in keyboard_config.h.
 * Inputs always use pull-up: active row is driven low, a pressed key reads 0
 * (assumed by KeyScanner).
 */

class KeyMatrix
{
    friend class System;   // Only System can create it (same pattern as UartDMAStdio)

public:
    enum class Direction : uint8_t
    {
        RowToCol = 0,   // Rows output, columns input (default)
        ColToRow = 1,   // Columns output, rows input
    };

    struct Config
    {
        uint8_t rows = 0;
        uint8_t cols = 0;
        const uint8_t* row_pins = nullptr;   // [rows]
        const uint8_t* col_pins = nullptr;   // [cols]
        const KeyCodes* keymap = nullptr;    // [rows][cols] row-major: key_at(r,c)
        Direction direction = Direction::RowToCol;   // Output side
    };

    uint8_t rows() const { return rows_; }
    uint8_t cols() const { return cols_; }
    uint8_t row_pin(uint8_t r) const { return row_pins_[r]; }
    uint8_t col_pin(uint8_t c) const { return col_pins_[c]; }
    KeyCodes key_at(uint8_t r, uint8_t c) const { return keymap_[r * cols_ + c]; }
    Direction direction() const { return direction_; }

private:
    explicit KeyMatrix(const Config& cfg);
    ~KeyMatrix() = default;
    KeyMatrix(const KeyMatrix&) = delete;
    KeyMatrix& operator=(const KeyMatrix&) = delete;
    KeyMatrix(KeyMatrix&&) = delete;
    KeyMatrix& operator=(KeyMatrix&&) = delete;

    uint8_t rows_ = 0;
    uint8_t cols_ = 0;
    const uint8_t* row_pins_ = nullptr;
    const uint8_t* col_pins_ = nullptr;
    const KeyCodes* keymap_ = nullptr;
    Direction direction_ = Direction::RowToCol;
};
