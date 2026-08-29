#pragma once

#include <stdint.h>

/* 测试观察点：记录最近一次通过 tud_hid_report / tud_hid_keyboard_report
 * 发送的报告内容，供测试断言。 */

extern uint8_t  g_last_report_id;
extern uint8_t  g_last_report[64];
extern uint16_t g_last_report_len;
extern uint32_t g_report_calls;

void tusb_stub_reset(void);
