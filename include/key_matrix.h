#pragma once

#include <cstdint>

#include "key_codes.h"

/* KeyMatrix：键盘矩阵的静态描述（行列数、引脚、键码表、扫描方向）。
 *
 * 只描述"接线"，不触碰硬件；GPIO 扫描与去抖由 KeyScanner 完成。
 * 数据在构造时注入、运行时只读；具体布局（键位表）在 system.cpp 定义。
 * 输入侧固定使用上拉（pullup）：激活行拉低、按下读 0（KeyScanner 假定）。
 */

class KeyMatrix
{
    friend class System;   // 仅 System 可创建（参照 UartDMAStdio 模式）

public:
    enum class Direction : uint8_t
    {
        RowToCol = 0,   // 行输出、列输入（默认）
        ColToRow = 1,   // 列输出、行输入
    };

    struct Config
    {
        uint8_t rows = 0;
        uint8_t cols = 0;
        const uint8_t* row_pins = nullptr;   // [rows]
        const uint8_t* col_pins = nullptr;   // [cols]
        const KeyCodes* keymap = nullptr;    // [rows][cols] 行主序：key_at(r,c)
        Direction direction = Direction::RowToCol;   // 输出侧
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
