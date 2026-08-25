#pragma once

#include <cstdint>

/* USB HID 键盘模块：TinyUSB 初始化、后台任务与 6KRO/NKRO 报告模式切换。
 *
 * 职责边界：
 *  - key_scan 只产出原始 KeyCodes（ks_pressed_basic/extended/internal），
 *    不关心报告格式；
 *  - 所有报告编码与模式状态都收在本模块内部，对外只暴露本接口；
 *  - 模式切换内部动作（KEY_P_NKRO_ON_OFF / KEY_P_NKRO / KEY_P_6KRO）
 *    由 usb_hid_task 内部消费 ks_pressed_internal 完成；
 *  - 其他模块（组合键、配置命令等）也可通过本接口主动切换模式。 */

// 初始化 TinyUSB（上电枚举为 HID 键盘）。stdio 初始化之后调用一次。
void usb_hid_init();

// 主循环每轮调用：tud_task() + 按当前模式编码并上报按键状态。
void usb_hid_task();

// --- 报告模式接口（6KRO <-> NKRO）---
bool usb_hid_nkro_enabled();        // 当前是否为 NKRO 模式
void usb_hid_set_nkro(bool enable); // 强制设定模式（切换时自动发空报告清状态）
void usb_hid_toggle_nkro();         // 翻转模式
