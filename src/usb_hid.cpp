/*
 * usb_hid.cpp
 * USB HID 键盘模块：TinyUSB 初始化、后台任务、报告编码与 6KRO/NKRO 模式切换。
 *
 * 职责边界：
 *  - key_scan 只产出原始 KeyCodes（ks_pressed_basic/extended/internal），
 *    不关心报告格式；
 *  - 本模块独占所有报告编码、模式状态与切换，对外只暴露 usb_hid.h 的接口；
 *  - 模式切换内部动作（KEY_P_NKRO_ON_OFF / KEY_P_NKRO / KEY_P_6KRO）
 *    在本模块内部消费 ks_pressed_internal 完成。
 */

#include "usb_hid.h"

#include <string.h>
#include <vector>
#include <algorithm>

#include "tusb.h"
#include "usb_descriptors.h"
#include "key_codes.h"
#include "key_scan.h"

/* ==================================================================== */
/* 内部状态（模块私有）                                                   */
/* ==================================================================== */

static bool nkro_mode = false;               // 当前模式：true=NKRO, false=6KRO
static bool last_report_is_nkro = false;     // 上次发送的报告格式（模式切换时强制重发）

static hid_keyboard_report_t last_report6 = {0};
static uint8_t last_nkro_report[2 + NKRO_BITMAP_SIZE] = {0};

/* ==================================================================== */
/* 报告编码（key_scan 产出的是原始 KeyCodes，这里才编码为 HID 报告）       */
/* ==================================================================== */

// 6KRO（Boot 键盘格式）：修饰键进 modifier 字节，其余键最多 6 个
static void build_report_6kro(const std::vector<KeyCodes>& pressed,
                              hid_keyboard_report_t& out) {
    out = {};
    uint8_t idx = 0;
    for (KeyCodes k : pressed) {
        if (k >= KEY_LEFT_CTRL && k <= KEY_RIGHT_GUI) {
            // 修饰键（0xE0~0xE7）→ modifier 字节
            out.modifier |= (uint8_t)(1u << (k - KEY_LEFT_CTRL));
        } else if (k > KEY_NULL && k <= KEY_RIGHT_GUI) {
            // 普通键：最多 6 个（超出丢弃，这是 6KRO 的语义）
            if (idx < 6) out.keycode[idx++] = (uint8_t)k;
        }
        // 扩展（KEY_E_*）/私有（KEY_P_*）不在此处理
    }
}

// NKRO（256 键位图）：modifier 字节 + 保留字节 + 32 字节位图
static void build_report_nkro(const std::vector<KeyCodes>& pressed,
                              uint8_t (&out)[2 + NKRO_BITMAP_SIZE]) {
    memset(out, 0, 2 + NKRO_BITMAP_SIZE);
    for (KeyCodes k : pressed) {
        if (k >= KEY_LEFT_CTRL && k <= KEY_RIGHT_GUI) {
            out[0] |= (uint8_t)(1u << (k - KEY_LEFT_CTRL));
        } else if (k > KEY_NULL && k <= KEY_RIGHT_GUI) {
            out[2 + (k >> 3)] |= (uint8_t)(1u << (k & 7));
        }
    }
}

/* ==================================================================== */
/* 上报（仅在变化时发送；模式切换后强制重发）                              */
/* ==================================================================== */

static void send_reports(const std::vector<KeyCodes>& pressed) {
    if (nkro_mode) {
        uint8_t nkro_report[2 + NKRO_BITMAP_SIZE];
        build_report_nkro(pressed, nkro_report);
        if (!last_report_is_nkro ||
            memcmp(nkro_report, last_nkro_report, sizeof(nkro_report)) != 0) {
            memcpy(last_nkro_report, nkro_report, sizeof(nkro_report));
            last_report_is_nkro = true;
            tud_hid_report(REPORT_ID_NKRO, nkro_report, sizeof(nkro_report));
        }
    } else {
        hid_keyboard_report_t report6;
        build_report_6kro(pressed, report6);
        if (last_report_is_nkro ||
            memcmp(&report6, &last_report6, sizeof(report6)) != 0) {
            last_report6 = report6;
            last_report_is_nkro = false;
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, report6.modifier,
                                    report6.keycode);
        }
    }
}

/* ==================================================================== */
/* 内部动作：KEY_P_* 私有键的按下沿处理（模式切换）                       */
/* ==================================================================== */

static void handle_internal_actions() {
    static std::vector<KeyCodes> prev_pressed;

    for (KeyCodes k : ks_pressed_internal) {
        if (std::find(prev_pressed.begin(), prev_pressed.end(), k)
                != prev_pressed.end()) {
            continue; // 之前已按下，不是上升沿
        }
        switch (k) {
            case KEY_P_NKRO_ON_OFF: usb_hid_toggle_nkro(); break;
            case KEY_P_NKRO:        usb_hid_set_nkro(true);  break;
            case KEY_P_6KRO:        usb_hid_set_nkro(false); break;
            default: break; // 其他私有动作暂未实现
        }
    }
    prev_pressed = ks_pressed_internal;
}

/* ==================================================================== */
/* 公开接口                                                              */
/* ==================================================================== */

void usb_hid_init() {
    tusb_init();
}

void usb_hid_task() {
    tud_task(); // TinyUSB 后台任务（枚举/中断）

    if (!tud_hid_ready()) {
        return;
    }

    handle_internal_actions();

#ifdef USB_HID_SIM_KEYS
    // 无矩阵时验证 HID 上报链路的模拟按键：周期性按下/释放 'A'
    static uint32_t tick = 0;
    if (((tick++) & 0x3F) < 32) {
        static const std::vector<KeyCodes> sim{ KEY_A };
        send_reports(sim);
    } else {
        send_reports({});
    }
#else
    send_reports(ks_pressed_basic);
#endif
}

bool usb_hid_nkro_enabled() {
    return nkro_mode;
}

void usb_hid_set_nkro(bool enable) {
    if (nkro_mode == enable) {
        return;
    }
    nkro_mode = enable;

    // 模式变化：立即发空报告，让主机清掉旧模式下的按键状态
    if (enable) {
        uint8_t empty[2 + NKRO_BITMAP_SIZE] = {0};
        tud_hid_report(REPORT_ID_NKRO, empty, sizeof(empty));
    } else {
        hid_keyboard_report_t empty6 = {0};
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, empty6.keycode);
    }
}

void usb_hid_toggle_nkro() {
    usb_hid_set_nkro(!nkro_mode);
}

/* ==================================================================== */
/* TinyUSB 回调（弱符号覆盖，需 extern "C" 与头文件声明保持一致）          */
/* ==================================================================== */
extern "C" {

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer, uint16_t reqlen) {
    (void)itf; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize) {
    (void)itf; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
    // TODO: 可在收到 OUTPUT 报告时驱动 Num/Caps/Scroll 指示灯
}

} // extern "C"
