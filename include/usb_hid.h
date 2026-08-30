#pragma once

#include <cstdint>
#include <vector>

#include "tusb.h"
#include "usb_descriptors.h"
#include "key_codes.h"

/* USB HID keyboard subsystem: TinyUSB init, background task, report encoding
 * and 6KRO/NKRO mode switching.
 *
 * Responsibility boundary:
 *  - KeyScanner only produces raw KeyCodes (pressed_basic/extended/internal)
 *    and knows nothing about report formats;
 *  - this class exclusively owns all report encoding, mode state and switching;
 *  - mode-switch internal actions (KEY_P_NKRO_ON_OFF / KEY_P_NKRO / KEY_P_6KRO)
 *    are consumed inside task();
 *  - other modules (combos, config commands, etc.) may also switch actively via
 *    set_nkro/toggle_nkro.
 *
 * Held by the System singleton, accessed as sys.hid (same pattern as sys.io).
 */

class KeyScanner;   // Forward declaration: only a reference is held

class UsbHid
{
    friend class System;

public:
    void init();            // tusb_init(): enumerate as HID keyboard at power-on; call once after stdio
    void task();            // Called each main-loop iteration: tud_task + encode & report per current mode

    bool nkro_enabled() const;    // Whether currently in NKRO mode
    void set_nkro(bool enable);   // Force the mode (sends an empty report to clear state on switch)
    void toggle_nkro();           // Flip the mode

private:
    UsbHid(const KeyScanner& scan);   // Dependency-injected scan results
    ~UsbHid() = default;
    UsbHid(const UsbHid&) = delete;
    UsbHid& operator=(const UsbHid&) = delete;
    UsbHid(UsbHid&&) = delete;
    UsbHid& operator=(UsbHid&&) = delete;

    // ---- Report encoding (raw KeyCodes -> HID report) ----
    void build_report_6kro(const std::vector<KeyCodes>& pressed,
                           hid_keyboard_report_t& out) const;
    void build_report_nkro(const std::vector<KeyCodes>& pressed,
                           uint8_t (&out)[2 + NKRO_BITMAP_SIZE]) const;
    void build_report_consumer(const std::vector<KeyCodes>& pressed,
                               uint8_t (&out)[CONSUMER_BITMAP_SIZE]) const;
    void build_report_system(const std::vector<KeyCodes>& pressed,
                             uint8_t (&out)[CONSUMER_SYS_BITMAP_SIZE]) const;
    void build_report_app(const std::vector<KeyCodes>& pressed,
                          uint8_t (&out)[CONSUMER_APP_BITMAP_SIZE]) const;
    static bool key_to_usage(KeyCodes key, uint16_t& usage);

    // ---- Sending (only on change) ----
    void send_reports(const std::vector<KeyCodes>& pressed);
    void send_consumer_report(const std::vector<KeyCodes>& pressed);
    void send_report_system(const std::vector<KeyCodes>& pressed);
    void send_report_app(const std::vector<KeyCodes>& pressed);

    // ---- Internal actions (KEY_P_* private-key press-edge handling) ----
    void handle_internal_actions();

    // ---- State ----
    bool nkro_mode_ = false;               // Current mode: true=NKRO, false=6KRO
    bool last_report_is_nkro_ = false;     // Format of last sent report (forced resend on switch)
    hid_keyboard_report_t last_report6_ = {};
    uint8_t last_nkro_report_[2 + NKRO_BITMAP_SIZE] = {};
    uint8_t last_consumer_report_[CONSUMER_BITMAP_SIZE] = {};
    uint8_t last_sys_report_[CONSUMER_SYS_BITMAP_SIZE] = {};
    uint8_t last_app_report_[CONSUMER_APP_BITMAP_SIZE] = {};
    std::vector<KeyCodes> prev_internal_;  // Internal-key press-edge detection

    const KeyScanner& scan_;              // Scan-result source (dependency injection)
};
