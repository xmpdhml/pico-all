#pragma once

/* 统一配置 —— 所有需要按硬件调整的定义集中在此文件。
 *
 *  一、引脚：UART / LED / ENCODER / 矩阵行列
 *  二、键位：矩阵规模 + 键位映射表 + 矩阵配置
 *
 * 改接线 / 键位 / 编码器只动这一个文件即可。
 * 键码定义见 key_codes.h。
 */

#include <cstdint>

#include "key_codes.h"
#include "key_matrix.h"

namespace keyboard_config {

/* ============ 一、引脚 ============ */

/* 调试 UART（IO，经 UartDMAStdio 转发 printf/DEBUG_LOG） */
inline constexpr uint8_t kUartTxPin = 0;   // GP0 = UART TX
inline constexpr uint8_t kUartRxPin = 1;   // GP1 = UART RX

/* 状态 LED（心跳指示） */
inline constexpr uint8_t kLedPin = 2;      // GP2

/* 旋转编码器（预留，暂未接线；按实际接线调整） */
inline constexpr uint8_t kEncoderAPin     = 29;   // A 相
inline constexpr uint8_t kEncoderBPin     = 30;   // B 相
inline constexpr uint8_t kEncoderButtonPin = 31;  // 按键

/* 按键矩阵 */
inline constexpr uint8_t kRows = 3;
inline constexpr uint8_t kCols = 3;

// 行引脚（输出，选通行，激活时拉低）
inline constexpr uint8_t kRowPins[kRows] = { 32, 34, 36 };

// 列引脚（输入，上拉；按下读到 0）
inline constexpr uint8_t kColPins[kCols] = { 38, 40, 42 };

/* ============ 二、键位 ============ */

// 键位映射表：kKeymap[row][col] -> KeyCodes
// 当前是 3×3 占位表（KEY_A..I）。
inline constexpr KeyCodes kKeymap[kRows][kCols] = {
    { KEY_A, KEY_B, KEY_C },
    { KEY_D, KEY_E, KEY_F },
    { KEY_G, KEY_H, KEY_I },
};

// 组装好的矩阵配置（System 构造时直接引用）
inline const KeyMatrix::Config kMatrixConfig = {
    .rows       = kRows,
    .cols       = kCols,
    .row_pins   = kRowPins,
    .col_pins   = kColPins,
    .keymap     = &kKeymap[0][0],
    .direction  = KeyMatrix::Direction::RowToCol,
};

} // namespace keyboard_config
