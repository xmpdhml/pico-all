#include "key_scan.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"   // sleep_us

KeyScanner::KeyScanner(const KeyMatrix& matrix)
    : matrix_(matrix)
{
    debounce_.assign(matrix_.rows() * matrix_.cols(), 0);
}

void KeyScanner::init()
{
    // 输出侧（行）：默认驱动到非激活电平
    const uint8_t inactive = matrix_.pullup() ? 1 : 0;
    for (uint8_t r = 0; r < matrix_.rows(); r++) {
        gpio_init(matrix_.row_pin(r));
        gpio_set_dir(matrix_.row_pin(r), GPIO_OUT);
        gpio_put(matrix_.row_pin(r), inactive);
    }

    // 输入侧（列）：上下拉
    for (uint8_t c = 0; c < matrix_.cols(); c++) {
        gpio_init(matrix_.col_pin(c));
        gpio_set_dir(matrix_.col_pin(c), GPIO_IN);
        if (matrix_.pullup()) {
            gpio_pull_up(matrix_.col_pin(c));
        } else {
            gpio_pull_down(matrix_.col_pin(c));
        }
    }
}

void KeyScanner::scan()
{
    pressed_basic_.clear();
    pressed_extended_.clear();
    pressed_internal_.clear();

    const bool pullup = matrix_.pullup();
    const uint8_t active = pullup ? 0 : 1;         // 激活行驱动电平
    const uint8_t pressed_level = pullup ? 0 : 1;  // 输入读到"按下"的电平

    for (uint8_t r = 0; r < matrix_.rows(); r++) {
        gpio_put(matrix_.row_pin(r), active);
        sleep_us(5);   // 电平稳定

        for (uint8_t c = 0; c < matrix_.cols(); c++) {
            const bool raw = (gpio_get(matrix_.col_pin(c)) == pressed_level);
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

        gpio_put(matrix_.row_pin(r), active ^ 1);  // 释放行（回非激活电平）
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
