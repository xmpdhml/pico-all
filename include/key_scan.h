#pragma once

#include <vector>

#include "key_codes.h"

// expect to have macros defined
// bool KS_USE_MATRIX
//      byte KS_ROWS, KS_COLS;
//      int KS_ROW_PINS[KS_ROWS];
//      int KS_COL_PINS[KS_COLS];
//      u_short keyscan_matrix[KS_ROWS][KS_COLS];
//      bool KS_MATRIX_ROW2COL;
// bool KS_USE_DIRECT
//      byte KS_DIRECTS;
//      int KS_DIRECT_PINS[KS_DIRECTS];
//      u_short keyscan_direct[KS_DIRECTS];
//      bool KS_DIRECT_PULLUP;
// bool KS_USE_ENCODER
//      int KS_ENCODER_A_PIN;
//      int KS_ENCODER_B_PIN;
//      int KS_ENCODER_PULLUP;
// u_short KS_NKRO_BEGIN;
// u_short KS_NKRO_END;
// u_short KS_DEBOUNCE_NS;

#ifdef KS_USE_MATRIX
    void ks_matrix();
#endif

#ifdef KS_USE_DIRECT
    void ks_direct();
#endif

#ifdef KS_USE_ENCODER
    void ks_encoder();
#endif

void ks_init();
void ks_scan();

extern std::vector<KeyCodes> ks_pressed_basic;
extern std::vector<KeyCodes> ks_pressed_extended;
extern std::vector<KeyCodes> ks_pressed_internal;
extern bool ks_pressed_basic_changed;
extern bool ks_pressed_extended_changed;
extern bool ks_pressed_internal_changed;
