#include "key_scan.h"

void ks_init()
{

}

void ks_scan()
{
    ks_pressed_basic.clear();
    ks_pressed_extended.clear();
    ks_pressed_internal.clear();
    ks_pressed_basic_changed = false;
    ks_pressed_extended_changed = false;
    ks_pressed_internal_changed = false;

#ifdef KS_USE_MATRIX
    ks_matrix();
#endif
#ifdef KS_USE_DIRECT
    ks_direct();
#endif
#ifdef KS_USE_ENCODER
    ks_encoder();
#endif
}

std::vector<KeyCodes> ks_pressed_basic;
std::vector<KeyCodes> ks_pressed_extended;
std::vector<KeyCodes> ks_pressed_internal;
bool ks_pressed_basic_changed;
bool ks_pressed_extended_changed;
bool ks_pressed_internal_changed;
