#pragma once

#include <stdint.h>

/* Test observer: records the latest report sent via tud_hid_report /
 * tud_hid_keyboard_report for test assertions. */

extern uint8_t  g_last_report_id;
extern uint8_t  g_last_report[64];
extern uint16_t g_last_report_len;
extern uint32_t g_report_calls;

void tusb_stub_reset(void);
