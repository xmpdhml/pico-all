#pragma once

#include <cstdint>
#include <vector>

#include "tusb.h"
#include "usb_descriptors.h"
#include "key_codes.h"

/* USB HID 键盘子系统：TinyUSB 初始化、后台任务、报告编码与 6KRO/NKRO 模式切换。
 *
 * 职责边界：
 *  - KeyScanner 只产出原始 KeyCodes（pressed_basic/extended/internal），
 *    不关心报告格式；
 *  - 本类独占所有报告编码、模式状态与切换；
 *  - 模式切换内部动作（KEY_P_NKRO_ON_OFF / KEY_P_NKRO / KEY_P_6KRO）
 *    由 task() 内部消费扫描结果完成；
 *  - 其他模块（组合键、配置命令等）也可通过 set_nkro/toggle_nkro 主动切换。
 *
 * 由 System 单例持有，经 sys.hid 访问（与 sys.io 同模式）。
 */

class KeyScanner;   // 前向声明：仅持有引用

class UsbHid
{
    friend class System;

public:
    void init();            // tusb_init()：上电枚举为 HID 键盘，stdio 初始化后调用一次
    void task();            // 主循环每轮调用：tud_task + 按当前模式编码并上报

    bool nkro_enabled() const;    // 当前是否为 NKRO 模式
    void set_nkro(bool enable);   // 强制设定模式（切换时自动发空报告清状态）
    void toggle_nkro();           // 翻转模式

private:
    UsbHid(const KeyScanner& scan);   // 依赖注入扫描结果
    ~UsbHid() = default;
    UsbHid(const UsbHid&) = delete;
    UsbHid& operator=(const UsbHid&) = delete;
    UsbHid(UsbHid&&) = delete;
    UsbHid& operator=(UsbHid&&) = delete;

    // ---- 报告编码（原始 KeyCodes -> HID 报告） ----
    void build_report_6kro(const std::vector<KeyCodes>& pressed,
                           hid_keyboard_report_t& out) const;
    void build_report_nkro(const std::vector<KeyCodes>& pressed,
                           uint8_t (&out)[2 + NKRO_BITMAP_SIZE]) const;
    void build_report_consumer(const std::vector<KeyCodes>& pressed,
                               uint8_t (&out)[CONSUMER_BITMAP_SIZE]) const;
    static bool key_to_consumer_usage(KeyCodes key, uint16_t& usage);

    // ---- 上报（仅在变化时发送） ----
    void send_reports(const std::vector<KeyCodes>& pressed);
    void send_consumer_report(const std::vector<KeyCodes>& pressed);

    // ---- 内部动作（KEY_P_* 私有键按下沿处理） ----
    void handle_internal_actions();

    // ---- 状态 ----
    bool nkro_mode_ = false;               // 当前模式：true=NKRO, false=6KRO
    bool last_report_is_nkro_ = false;     // 上次发送的报告格式（模式切换强制重发）
    hid_keyboard_report_t last_report6_ = {};
    uint8_t last_nkro_report_[2 + NKRO_BITMAP_SIZE] = {};
    uint8_t last_consumer_report_[CONSUMER_BITMAP_SIZE] = {};
    std::vector<KeyCodes> prev_internal_;  // 内部键按下沿检测

    const KeyScanner& scan_;              // 扫描结果来源（依赖注入）
};
