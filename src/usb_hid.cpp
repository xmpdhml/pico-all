/*
 * usb_hid.cpp
 * USB HID 键盘子系统（UsbHid）：TinyUSB 初始化、后台任务、报告编码与
 * 6KRO/NKRO 模式切换。
 *
 * 职责边界：
 *  - KeyScanner 只产出原始 KeyCodes（pressed_basic/extended/internal），
 *    不关心报告格式；
 *  - 本类独占所有报告编码、模式状态与切换；
 *  - 模式切换内部动作（KEY_P_NKRO_ON_OFF / KEY_P_NKRO / KEY_P_6KRO）
 *    在 task() 内部消费扫描结果完成。
 *  - 由 System 单例持有（sys.hid），见 system.h/system.cpp。
 */

#include "usb_hid.h"

#include <string.h>
#include <algorithm>

#include "tusb.h"
#include "usb_descriptors.h"
#include "key_codes.h"
#include "key_scan.h"

/* ==================================================================== */
/* 构造                                                                  */
/* ==================================================================== */

UsbHid::UsbHid(const KeyScanner& scan)
    : scan_(scan)
{
}

/* ==================================================================== */
/* 公开接口                                                              */
/* ==================================================================== */

void UsbHid::init() {
    tusb_init();
}

void UsbHid::task() {
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
    send_reports(scan_.pressed_basic());            // 6KRO / NKRO 键盘
    send_consumer_report(scan_.pressed_extended()); // Consumer 媒体（位图）
    send_report_system(scan_.pressed_extended());   // Consumer 系统键（位图）
    send_report_app(scan_.pressed_extended());      // Consumer 应用启动（位图）
#endif
}

bool UsbHid::nkro_enabled() const {
    return nkro_mode_;
}

void UsbHid::set_nkro(bool enable) {
    if (nkro_mode_ == enable) {
        return;
    }
    nkro_mode_ = enable;

    // 模式变化：立即发空报告，让主机清掉旧模式下的按键状态
    if (enable) {
        uint8_t empty[2 + NKRO_BITMAP_SIZE] = {0};
        tud_hid_report(REPORT_ID_NKRO, empty, sizeof(empty));
    } else {
        hid_keyboard_report_t empty6 = {0};
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, empty6.keycode);
    }
}

void UsbHid::toggle_nkro() {
    set_nkro(!nkro_mode_);
}

/* ==================================================================== */
/* 内部动作：KEY_P_* 私有键的按下沿处理（模式切换）                       */
/* ==================================================================== */

void UsbHid::handle_internal_actions() {
    for (KeyCodes k : scan_.pressed_internal()) {
        if (std::find(prev_internal_.begin(), prev_internal_.end(), k)
                != prev_internal_.end()) {
            continue; // 之前已按下，不是上升沿
        }
        switch (k) {
            case KEY_P_NKRO_ON_OFF: toggle_nkro(); break;
            case KEY_P_NKRO:        set_nkro(true);  break;
            case KEY_P_6KRO:        set_nkro(false); break;
            default: break; // 其他私有动作暂未实现
        }
    }
    prev_internal_ = scan_.pressed_internal();
}

/* ==================================================================== */
/* 报告编码（key_scan 产出的是原始 KeyCodes，这里才编码为 HID 报告）       */
/* ==================================================================== */

void UsbHid::build_report_6kro(const std::vector<KeyCodes>& pressed,
                               hid_keyboard_report_t& out) const {
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

void UsbHid::build_report_nkro(const std::vector<KeyCodes>& pressed,
                               uint8_t (&out)[2 + NKRO_BITMAP_SIZE]) const {
    memset(out, 0, 2 + NKRO_BITMAP_SIZE);
    for (KeyCodes k : pressed) {
        if (k >= KEY_LEFT_CTRL && k <= KEY_RIGHT_GUI) {
            out[0] |= (uint8_t)(1u << (k - KEY_LEFT_CTRL));
        } else if (k > KEY_NULL && k <= KEY_RIGHT_GUI) {
            out[2 + (k >> 3)] |= (uint8_t)(1u << (k & 7));
        }
    }
}

bool UsbHid::key_to_usage(KeyCodes key, uint16_t& usage) {
    switch (key) {
        // 系统键（0x30~0x32）
        case KEY_E_POWER: usage = 0x0030; return true;
        case KEY_E_RESET: usage = 0x0031; return true;
        case KEY_E_SLEEP: usage = 0x0032; return true;
        // 媒体（0xB0~0xEF）
        case KEY_E_PLAY_PAUSE: usage = 0x00CD; return true;
        case KEY_E_STOP:       usage = 0x00B7; return true;
        case KEY_E_NEXT_TRACK: usage = 0x00B5; return true;
        case KEY_E_PREV_TRACK: usage = 0x00B6; return true;
        case KEY_E_MUTE:       usage = 0x00E2; return true;
        case KEY_E_VOL_INC:    usage = 0x00E9; return true;
        case KEY_E_VOL_DEC:    usage = 0x00EA; return true;
        default: break;
    }
    // 应用启动 AL_*：枚举顺序即 usage 顺序（自 0x184 起偏移），覆盖全部
    // KEY_E_AL_*；个别键若需精确 usage，可在上面补显式 case。
    if (key >= KEY_E_AL_WORD && key <= KEY_E_AL_AUDIO_PLAYER) {
        usage = (uint16_t)(CONSUMER_APP_USAGE_MIN + (key - KEY_E_AL_WORD));
        return true;
    }
    return false;
}

void UsbHid::build_report_consumer(const std::vector<KeyCodes>& pressed,
                                   uint8_t (&out)[CONSUMER_BITMAP_SIZE]) const {
    memset(out, 0, CONSUMER_BITMAP_SIZE);
    for (KeyCodes k : pressed) {
        uint16_t usage;
        if (!key_to_usage(k, usage)) continue;
        if (usage < CONSUMER_USAGE_MIN || usage > CONSUMER_USAGE_MAX) continue;
        uint16_t bit = usage - CONSUMER_USAGE_MIN; // 0..63
        out[bit >> 3] |= (uint8_t)(1u << (bit & 7));
    }
}

void UsbHid::build_report_system(const std::vector<KeyCodes>& pressed,
                                 uint8_t (&out)[CONSUMER_SYS_BITMAP_SIZE]) const {
    memset(out, 0, CONSUMER_SYS_BITMAP_SIZE);
    for (KeyCodes k : pressed) {
        uint16_t usage;
        if (!key_to_usage(k, usage)) continue;
        if (usage < CONSUMER_SYS_USAGE_MIN || usage > CONSUMER_SYS_USAGE_MAX) continue;
        uint16_t bit = usage - CONSUMER_SYS_USAGE_MIN; // 0..2
        out[bit >> 3] |= (uint8_t)(1u << (bit & 7));
    }
}

void UsbHid::build_report_app(const std::vector<KeyCodes>& pressed,
                              uint8_t (&out)[CONSUMER_APP_BITMAP_SIZE]) const {
    memset(out, 0, CONSUMER_APP_BITMAP_SIZE);
    for (KeyCodes k : pressed) {
        uint16_t usage;
        if (!key_to_usage(k, usage)) continue;
        if (usage < CONSUMER_APP_USAGE_MIN || usage > CONSUMER_APP_USAGE_MAX) continue;
        uint16_t bit = usage - CONSUMER_APP_USAGE_MIN; // 0..66
        out[bit >> 3] |= (uint8_t)(1u << (bit & 7));
    }
}

/* ==================================================================== */
/* 上报（仅在变化时发送；模式切换后强制重发）                              */
/* ==================================================================== */

void UsbHid::send_reports(const std::vector<KeyCodes>& pressed) {
    if (nkro_mode_) {
        uint8_t nkro_report[2 + NKRO_BITMAP_SIZE];
        build_report_nkro(pressed, nkro_report);
        if (!last_report_is_nkro_ ||
            memcmp(nkro_report, last_nkro_report_, sizeof(nkro_report)) != 0) {
            memcpy(last_nkro_report_, nkro_report, sizeof(nkro_report));
            last_report_is_nkro_ = true;
            tud_hid_report(REPORT_ID_NKRO, nkro_report, sizeof(nkro_report));
        }
    } else {
        hid_keyboard_report_t report6;
        build_report_6kro(pressed, report6);
        if (last_report_is_nkro_ ||
            memcmp(&report6, &last_report6_, sizeof(report6)) != 0) {
            last_report6_ = report6;
            last_report_is_nkro_ = false;
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, report6.modifier,
                                    report6.keycode);
        }
    }
}

void UsbHid::send_consumer_report(const std::vector<KeyCodes>& pressed) {
    uint8_t consumer_report[CONSUMER_BITMAP_SIZE];
    build_report_consumer(pressed, consumer_report);
    if (memcmp(consumer_report, last_consumer_report_,
               sizeof(consumer_report)) != 0) {
        memcpy(last_consumer_report_, consumer_report,
               sizeof(consumer_report));
        tud_hid_report(REPORT_ID_CONSUMER, consumer_report,
                       sizeof(consumer_report));
    }
}

void UsbHid::send_report_system(const std::vector<KeyCodes>& pressed) {
    uint8_t sys_report[CONSUMER_SYS_BITMAP_SIZE];
    build_report_system(pressed, sys_report);
    if (memcmp(sys_report, last_sys_report_, sizeof(sys_report)) != 0) {
        memcpy(last_sys_report_, sys_report, sizeof(sys_report));
        tud_hid_report(REPORT_ID_CONSUMER_SYS, sys_report, sizeof(sys_report));
    }
}

void UsbHid::send_report_app(const std::vector<KeyCodes>& pressed) {
    uint8_t app_report[CONSUMER_APP_BITMAP_SIZE];
    build_report_app(pressed, app_report);
    if (memcmp(app_report, last_app_report_, sizeof(app_report)) != 0) {
        memcpy(last_app_report_, app_report, sizeof(app_report));
        tud_hid_report(REPORT_ID_CONSUMER_APP, app_report, sizeof(app_report));
    }
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
