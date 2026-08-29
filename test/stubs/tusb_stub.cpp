#include "tusb.h"
#include "tusb_stub.h"

#include <string.h>

uint8_t  g_last_report_id = 0;
uint8_t  g_last_report[64] = {0};
uint16_t g_last_report_len = 0;
uint32_t g_report_calls = 0;

void tusb_stub_reset(void) {
    g_last_report_id = 0;
    memset(g_last_report, 0, sizeof(g_last_report));
    g_last_report_len = 0;
    g_report_calls = 0;
}

void tusb_init(void) {}
void tud_task(void) {}
bool tud_hid_ready(void) { return true; }

bool tud_hid_report(uint8_t report_id, void const* report, uint16_t len) {
    if (len > sizeof(g_last_report)) len = sizeof(g_last_report);
    g_last_report_id = report_id;
    memcpy(g_last_report, report, len);
    g_last_report_len = len;
    g_report_calls++;
    return true;
}

bool tud_hid_keyboard_report(uint8_t report_id, uint8_t modifier, uint8_t keycode[6]) {
    uint8_t buf[8] = {
        modifier, 0,
        keycode[0], keycode[1], keycode[2],
        keycode[3], keycode[4], keycode[5],
    };
    g_last_report_id = report_id;
    memcpy(g_last_report, buf, sizeof(buf));
    g_last_report_len = sizeof(buf);
    g_report_calls++;
    return true;
}
