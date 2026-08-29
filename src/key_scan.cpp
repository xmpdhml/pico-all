#include "key_scan.h"

KeyScanner::KeyScanner(const KeyMatrix& matrix, MatrixIO& io)
    : matrix_(matrix), io_(io)
{
    debounce_.assign(matrix_.rows() * matrix_.cols(), 0);
}

void KeyScanner::init()
{
    io_.init();
}

void KeyScanner::scan()
{
    pressed_basic_.clear();
    pressed_extended_.clear();
    pressed_internal_.clear();

    for (uint8_t r = 0; r < matrix_.rows(); r++) {
        for (uint8_t c = 0; c < matrix_.cols(); c++) {
            const bool raw = io_.read_cell(r, c);
            const size_t idx = (size_t)r * matrix_.cols() + c;

            // 每键计数去抖：连续按下/释放超过阈值才算稳定
            uint8_t& cnt = debounce_[idx];
            if (raw) {
                if (cnt < 2 * kDebounceThreshold) cnt++;
            } else {
                if (cnt > 0) cnt--;
            }

            if (cnt > kDebounceThreshold) {
                add_pressed(matrix_.key_at(r, c));
            }
        }
    }

    // 变化标志
    basic_changed_ = (pressed_basic_ != last_basic_);
    extended_changed_ = (pressed_extended_ != last_extended_);
    internal_changed_ = (pressed_internal_ != last_internal_);
    last_basic_ = pressed_basic_;
    last_extended_ = pressed_extended_;
    last_internal_ = pressed_internal_;
}

void KeyScanner::add_pressed(KeyCodes code)
{
    if (code == KEY_NULL) {
        return;
    }
    if (code <= KEY_RIGHT_GUI) {
        pressed_basic_.push_back(code);
    } else if (code >= KEY_E_BEGIN && code < KEY_P_BEGIN) {
        pressed_extended_.push_back(code);
    } else if (code >= KEY_P_BEGIN) {
        pressed_internal_.push_back(code);
    }
}
