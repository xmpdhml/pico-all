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
    // 输出侧（行）：默认驱动到高电平（非激活）
    for (uint8_t r = 0; r < matrix_.rows(); r++) {
        gpio_init(matrix_.row_pin(r));
        gpio_set_dir(matrix_.row_pin(r), GPIO_OUT);
        gpio_put(matrix_.row_pin(r), 1);
    }

    // 输入侧（列）：统一上拉（只支持 pullup 方案）
    for (uint8_t c = 0; c < matrix_.cols(); c++) {
        gpio_init(matrix_.col_pin(c));
        gpio_set_dir(matrix_.col_pin(c), GPIO_IN);
        gpio_pull_up(matrix_.col_pin(c));
    }
}

void KeyScanner::scan()
{
    pressed_basic_.clear();
    pressed_extended_.clear();
    pressed_internal_.clear();

    // 只支持 pullup：激活行拉低（0），按下时列读到 0
    const uint8_t active = 0;

    for (uint8_t r = 0; r < matrix_.rows(); r++) {
        gpio_put(matrix_.row_pin(r), active);
        sleep_us(5);   // 电平稳定

        for (uint8_t c = 0; c < matrix_.cols(); c++) {
            const bool raw = (gpio_get(matrix_.col_pin(c)) == 0);
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

        gpio_put(matrix_.row_pin(r), 1);  // 释放行（回高电平）
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
