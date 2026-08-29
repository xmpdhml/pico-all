#define private public   // 测试需要访问私有构造
#include "key_scan.h"    // 连带 key_matrix.h / matrix_io.h
#undef private

#include "harness.h"

#include <vector>

namespace {

// 模拟矩阵：可编程设置每个格的"按下/释放"
class SimMatrixIO : public MatrixIO {
public:
    SimMatrixIO(const KeyMatrix& m)
        : matrix_(m), cells_(m.rows() * m.cols(), false) {}
    void init() override {}
    bool read_cell(uint8_t row, uint8_t col) const override {
        return cells_[(size_t)row * matrix_.cols() + col];
    }
    void set(uint8_t row, uint8_t col, bool pressed) {
        cells_[(size_t)row * matrix_.cols() + col] = pressed;
    }

private:
    const KeyMatrix& matrix_;
    std::vector<bool> cells_;
};

// 3×3：basic / extended / internal / modifier / null 混排
const uint8_t kRowsP[] = { 1, 2, 3 };
const uint8_t kColsP[] = { 4, 5, 6 };
const KeyCodes kMap[3][3] = {
    { KEY_A,           KEY_E_VOL_INC,     KEY_P_NKRO_ON_OFF },
    { KEY_LEFT_CTRL,   KEY_NULL,          KEY_B },
    { KEY_C,           KEY_D,             KEY_E },
};
const KeyMatrix::Config kCfg = {
    .rows       = 3,
    .cols       = 3,
    .row_pins   = kRowsP,
    .col_pins   = kColsP,
    .keymap     = &kMap[0][0],
    .direction  = KeyMatrix::Direction::RowToCol,
};

} // namespace

UT_CASE(scan_nothing_pressed) {
    KeyMatrix m(kCfg);
    SimMatrixIO io(m);
    KeyScanner ks(m, io);
    ks.scan();
    CHECK(ks.pressed_basic().empty());
    CHECK(ks.pressed_extended().empty());
    CHECK(ks.pressed_internal().empty());
    CHECK(!ks.basic_changed());
}

// 记录当前去抖语义（计数阈值 4，需 5 次扫描才上报按下）。
// TODO: 切换到 QMK eager/defer 后，本用例改为"按下立即上报、释放延迟"。
UT_CASE(scan_press_requires_debounce) {
    KeyMatrix m(kCfg);
    SimMatrixIO io(m);
    KeyScanner ks(m, io);
    io.set(0, 0, true);   // KEY_A
    for (int i = 0; i < 4; i++) {
        ks.scan();
        CHECK(ks.pressed_basic().empty());
    }
    ks.scan();   // 第 5 次 → 上报
    CHECK_EQ((int)ks.pressed_basic().size(), 1);
    CHECK_EQ(ks.pressed_basic()[0], KEY_A);
    CHECK(ks.basic_changed());
}

UT_CASE(scan_classify_ranges) {
    KeyMatrix m(kCfg);
    SimMatrixIO io(m);
    KeyScanner ks(m, io);
    io.set(0, 1, true);   // KEY_E_VOL_INC → extended
    io.set(0, 2, true);   // KEY_P_NKRO_ON_OFF → internal
    io.set(1, 0, true);   // KEY_LEFT_CTRL → basic（修饰键）
    for (int i = 0; i < 5; i++) ks.scan();
    CHECK_EQ((int)ks.pressed_basic().size(), 1);
    CHECK_EQ(ks.pressed_basic()[0], KEY_LEFT_CTRL);
    CHECK_EQ((int)ks.pressed_extended().size(), 1);
    CHECK_EQ(ks.pressed_extended()[0], KEY_E_VOL_INC);
    CHECK_EQ((int)ks.pressed_internal().size(), 1);
    CHECK_EQ(ks.pressed_internal()[0], KEY_P_NKRO_ON_OFF);
}

UT_CASE(scan_null_key_ignored) {
    KeyMatrix m(kCfg);
    SimMatrixIO io(m);
    KeyScanner ks(m, io);
    io.set(1, 1, true);   // KEY_NULL
    for (int i = 0; i < 5; i++) ks.scan();
    CHECK(ks.pressed_basic().empty());
    CHECK(ks.pressed_extended().empty());
    CHECK(ks.pressed_internal().empty());
}

UT_CASE(scan_release_after_hold) {
    KeyMatrix m(kCfg);
    SimMatrixIO io(m);
    KeyScanner ks(m, io);
    io.set(0, 0, true);            // KEY_A 按住
    for (int i = 0; i < 8; i++) ks.scan();   // 计数封顶 8
    CHECK_EQ((int)ks.pressed_basic().size(), 1);
    io.set(0, 0, false);           // 释放
    for (int i = 0; i < 3; i++) {  // 计数 8→7→6→5 仍为按下
        ks.scan();
        CHECK_EQ((int)ks.pressed_basic().size(), 1);
    }
    ks.scan();                     // 计数 5→4 → 消失
    CHECK(ks.pressed_basic().empty());
    CHECK(ks.basic_changed());
}
