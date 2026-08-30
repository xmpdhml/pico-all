#define private public   // Tests need access to the private constructor
#include "key_matrix.h"
#undef private

#include "harness.h"

namespace {

const uint8_t kRowsP[] = { 10, 11, 12 };
const uint8_t kColsP[] = { 20, 21, 22 };
const KeyCodes kMap[3][3] = {
    { KEY_A, KEY_B, KEY_C },
    { KEY_D, KEY_E, KEY_F },
    { KEY_G, KEY_H, KEY_I },
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

UT_CASE(key_matrix_geometry) {
    KeyMatrix m(kCfg);
    CHECK_EQ(m.rows(), 3u);
    CHECK_EQ(m.cols(), 3u);
    CHECK_EQ(m.row_pin(0), 10u);
    CHECK_EQ(m.row_pin(2), 12u);
    CHECK_EQ(m.col_pin(1), 21u);
    CHECK_EQ(m.key_at(0, 0), KEY_A);
    CHECK_EQ(m.key_at(2, 2), KEY_I);
    CHECK_EQ(m.key_at(1, 1), KEY_E);
}

UT_CASE(key_matrix_direction) {
    KeyMatrix m(kCfg);
    CHECK(m.direction() == KeyMatrix::Direction::RowToCol);
}
