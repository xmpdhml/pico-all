#pragma once

#include <vector>

#include "key_matrix.h"
#include "matrix_io.h"

/* KeyScanner：矩阵扫描 + 去抖引擎。
 *
 * - init()：初始化矩阵（转发给 MatrixIO）；
 * - scan()：逐格读取 + 每键计数去抖，产出三类按键列表
 *   （basic/extended/internal）及其变化标志，每轮主循环调用一次。
 *
 * 硬件访问（行驱动/列读取/上拉）通过 MatrixIO 注入，因此本类可在宿主机
 * 单元测试中配合模拟矩阵（SimMatrixIO）验证去抖与分类逻辑。
 *
 * 去抖：连续 kDebounceThreshold 次扫描读到同一状态才算稳定，
 * 阈值对应约 4×扫描周期（主循环 10ms 时约 40ms）。
 * 键位表与引脚来自 KeyMatrix（本类只负责"怎么扫"，不关心"扫出什么键"）。
 */

class KeyScanner
{
    friend class System;   // 仅 System 可创建（参照 UartDMAStdio 模式）

public:
    void init();   // 初始化矩阵（转发给 MatrixIO）
    void scan();   // 扫描 + 去抖（每轮主循环调用）

    // 本帧结果
    const std::vector<KeyCodes>& pressed_basic() const    { return pressed_basic_; }
    const std::vector<KeyCodes>& pressed_extended() const { return pressed_extended_; }
    const std::vector<KeyCodes>& pressed_internal() const { return pressed_internal_; }
    bool basic_changed() const    { return basic_changed_; }
    bool extended_changed() const { return extended_changed_; }
    bool internal_changed() const { return internal_changed_; }

private:
    KeyScanner(const KeyMatrix& matrix, MatrixIO& io);
    ~KeyScanner() = default;
    KeyScanner(const KeyScanner&) = delete;
    KeyScanner& operator=(const KeyScanner&) = delete;
    KeyScanner(KeyScanner&&) = delete;
    KeyScanner& operator=(KeyScanner&&) = delete;

    static constexpr uint8_t kDebounceThreshold = 4;

    void add_pressed(KeyCodes code);

    const KeyMatrix& matrix_;
    MatrixIO&        io_;
    std::vector<uint8_t> debounce_;          // [rows*cols] 去抖计数
    std::vector<KeyCodes> last_basic_;       // 上一帧（用于变化标志）
    std::vector<KeyCodes> last_extended_;
    std::vector<KeyCodes> last_internal_;

    std::vector<KeyCodes> pressed_basic_;
    std::vector<KeyCodes> pressed_extended_;
    std::vector<KeyCodes> pressed_internal_;
    bool basic_changed_ = false;
    bool extended_changed_ = false;
    bool internal_changed_ = false;
};
