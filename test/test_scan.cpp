#define private public   // Tests need access to the private constructor
#include "key_scan.h"    // Pulls in key_matrix.h / matrix_io.h
#undef private

#include "harness.h"

#include <vector>

namespace {

// Simulated matrix: each cell's "pressed/released" state is programmable
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

// 3×3: basic / extended / internal / modifier / null mixed
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

// Records the current debounce semantics (counting threshold 4, needs 5 scans before reporting a press).
// TODO: after switching to QMK eager/defer, change this case to "report on press immediately, delay on release".
UT_CASE(scan_press_requires_debounce) {
    KeyMatrix m(kCfg);
    SimMatrixIO io(m);
    KeyScanner ks(m, io);
    io.set(0, 0, true);   // KEY_A
    for (int i = 0; i < 4; i++) {
        ks.scan();
        CHECK(ks.pressed_basic().empty());
    }
    ks.scan();   // 5th scan → reported
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
    io.set(1, 0, true);   // KEY_LEFT_CTRL → basic (modifier)
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
    io.set(0, 0, true);            // KEY_A held
    for (int i = 0; i < 8; i++) ks.scan();   // Counter saturates at 8
    CHECK_EQ((int)ks.pressed_basic().size(), 1);
    io.set(0, 0, false);           // Release
    for (int i = 0; i < 3; i++) {  // Counter 8→7→6→5 still pressed
        ks.scan();
        CHECK_EQ((int)ks.pressed_basic().size(), 1);
    }
    ks.scan();                     // Counter 5→4 → disappears
    CHECK(ks.pressed_basic().empty());
    CHECK(ks.basic_changed());
}
