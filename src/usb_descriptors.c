/*
 * usb_descriptors.c
 * TinyUSB descriptors for the RP2350B USB HID keyboard (dual report: 6KRO + NKRO).
 *
 * Reference: Pico SDK examples / lib/tinyusb/examples/device/hid_composite
 */

#include <string.h>

#include "tusb.h"
#include "usb_descriptors.h"
#include "pico/unique_id.h"

/* ------------------------------------------------------------------ */
/* Device Descriptor                                                   */
/* ------------------------------------------------------------------ */
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = 0x0001,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 3,

    .bNumConfigurations = 0x01
};

/* ------------------------------------------------------------------ */
/* HID Report Descriptor (dual report: 6KRO + NKRO)                    */
/*   Report ID 1 = 6KRO: Boot keyboard format (modifier + 6 keys + LED output) */
/*   Report ID 2 = NKRO: 256-key bitmap (modifier + reserved + 32-byte bitmap) */
/* The firmware sends only one of them per current mode (see nkro_mode in usb_hid.cpp). */
/* ------------------------------------------------------------------ */
uint8_t const desc_hid_report[] = {
    /* ================= 6KRO (Boot keyboard format) ================= */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ),
    HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_KEYBOARD )
      /* 8 modifier bits (Ctrl/Shift/Alt/Gui) */
      HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ),
        HID_USAGE_MIN    ( 224 ),
        HID_USAGE_MAX    ( 231 ),
        HID_LOGICAL_MIN  ( 0 ),
        HID_LOGICAL_MAX  ( 1 ),
        HID_REPORT_COUNT ( 8 ),
        HID_REPORT_SIZE  ( 1 ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
        /* 8 reserved bits */
        HID_REPORT_COUNT ( 1 ),
        HID_REPORT_SIZE  ( 8 ),
        HID_INPUT        ( HID_CONSTANT ),
      /* 5-bit LED output (Num/Caps/Scroll/Kana/Compose) */
      HID_USAGE_PAGE ( HID_USAGE_PAGE_LED ),
        HID_USAGE_MIN    ( 1 ),
        HID_USAGE_MAX    ( 5 ),
        HID_REPORT_COUNT ( 5 ),
        HID_REPORT_SIZE  ( 1 ),
        HID_OUTPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
        HID_REPORT_COUNT ( 1 ),
        HID_REPORT_SIZE  ( 3 ),
        HID_OUTPUT       ( HID_CONSTANT ),
      /* 6-byte keycode array */
      HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ),
        HID_USAGE_MIN    ( 0 ),
        HID_USAGE_MAX_N  ( 255, 2 ),
        HID_LOGICAL_MIN  ( 0 ),
        HID_LOGICAL_MAX_N( 255, 2 ),
        HID_REPORT_COUNT ( 6 ),
        HID_REPORT_SIZE  ( 8 ),
        HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ),
    HID_COLLECTION_END,

    /* ================= NKRO (256-key bitmap) ================= */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ),
    HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_NKRO )
      HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ),
        HID_USAGE_MIN    ( 224 ),
        HID_USAGE_MAX    ( 231 ),
        HID_LOGICAL_MIN  ( 0 ),
        HID_LOGICAL_MAX  ( 1 ),
        HID_REPORT_COUNT ( 8 ),
        HID_REPORT_SIZE  ( 1 ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
        HID_REPORT_COUNT ( 1 ),
        HID_REPORT_SIZE  ( 8 ),
        HID_INPUT        ( HID_CONSTANT ),
        /* 256-bit keycode bitmap (32 bytes) */
        HID_USAGE_MIN    ( 0 ),
        HID_USAGE_MAX_N  ( 255, 2 ),
        HID_LOGICAL_MIN  ( 0 ),
        HID_LOGICAL_MAX  ( 1 ),
        HID_REPORT_COUNT_N ( 256, 2 ),
        HID_REPORT_SIZE  ( 1 ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
    HID_COLLECTION_END,

    /* ============ Consumer / media control (bitmap, multi-key per report) ============ */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ),
    HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_CONSUMER )
      /* 64-bit bitmap: bit N = usage (CONSUMER_USAGE_MIN + N), 1 bit per key */
      HID_LOGICAL_MIN ( 0 ),
      HID_LOGICAL_MAX ( 1 ),
      HID_USAGE_MIN   ( CONSUMER_USAGE_MIN ),
      HID_USAGE_MAX   ( CONSUMER_USAGE_MAX ),
      HID_REPORT_COUNT( (CONSUMER_USAGE_MAX - CONSUMER_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
    HID_COLLECTION_END,

    /* ============ Consumer / system keys Power/Reset/Sleep (bitmap) ============ */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ),
    HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_CONSUMER_SYS )
      /* 3-bit bitmap: bit N = usage (CONSUMER_SYS_USAGE_MIN + N) */
      HID_LOGICAL_MIN ( 0 ),
      HID_LOGICAL_MAX ( 1 ),
      HID_USAGE_MIN   ( CONSUMER_SYS_USAGE_MIN ),
      HID_USAGE_MAX   ( CONSUMER_SYS_USAGE_MAX ),
      HID_REPORT_COUNT( (CONSUMER_SYS_USAGE_MAX - CONSUMER_SYS_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
      /* Pad the bitmap up to a whole byte (3 data bits + 5 constant bits = 8 bits).
       * Windows validates the report descriptor and rejects the entire device with
       * Code 10 / "the report is not byte-aligned" if any report is not a multiple
       * of 8 bits. The padding count is derived from CONSUMER_SYS_BITMAP_SIZE — the
       * very size the firmware transmits — so descriptor and firmware cannot drift
       * apart. Constant bits carry no data and are ignored by the host. */
      HID_REPORT_COUNT( CONSUMER_SYS_BITMAP_SIZE * 8
                        - (CONSUMER_SYS_USAGE_MAX - CONSUMER_SYS_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_CONSTANT ),
    HID_COLLECTION_END,

    /* ============ Consumer / application launch AL_* (bitmap) ============ */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ),
    HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_CONSUMER_APP )
      /* 67-bit bitmap: bit N = usage (CONSUMER_APP_USAGE_MIN + N) (2-byte for usage > 255) */
      HID_LOGICAL_MIN ( 0 ),
      HID_LOGICAL_MAX ( 1 ),
      HID_USAGE_MIN_N ( CONSUMER_APP_USAGE_MIN, 2 ),
      HID_USAGE_MAX_N ( CONSUMER_APP_USAGE_MAX, 2 ),
      HID_REPORT_COUNT( (CONSUMER_APP_USAGE_MAX - CONSUMER_APP_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
      /* Pad to a whole byte: 67 data bits + 5 constant bits = 72 bits = 9 bytes
       * (CONSUMER_APP_BITMAP_SIZE). See the system-keys report above for why this
       * is required — Windows rejects a non-byte-aligned report outright. */
      HID_REPORT_COUNT( CONSUMER_APP_BITMAP_SIZE * 8
                        - (CONSUMER_APP_USAGE_MAX - CONSUMER_APP_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_CONSTANT ),
    HID_COLLECTION_END,
};

/* Windows requires every HID report to be a whole number of bytes; a report width
 * that does not fit its bitmap would make the padding above negative (and the
 * descriptor invalid). Guard the two bit-field reports at compile time. */
_Static_assert( ( CONSUMER_SYS_USAGE_MAX - CONSUMER_SYS_USAGE_MIN + 1 )
                    <= CONSUMER_SYS_BITMAP_SIZE * 8,
                "consumer system-keys report does not fit its bitmap" );
_Static_assert( ( CONSUMER_APP_USAGE_MAX - CONSUMER_APP_USAGE_MIN + 1 )
                    <= CONSUMER_APP_BITMAP_SIZE * 8,
                "consumer application-launch report does not fit its bitmap" );

/* ------------------------------------------------------------------ */
/* String Descriptors                                                  */
/* ------------------------------------------------------------------ */
/* Serial number: taken from the chip's unique board ID instead of a fixed
 * string. Windows keys its cached device state on VID/PID/serial, so a unique
 * serial makes every board a distinct device. This matters during bring-up:
 * if the device previously enumerated in a broken state (e.g. the USB
 * enumeration failure that showed up as Code 43), a fixed serial makes Windows
 * keep reusing that cached device node, so the board can still look broken even
 * after the firmware is fixed. Filled by usb_descriptors_init(). */
static char serial_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = "000000";

char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: Language ID = English
    "MyKeyboard",               // 1: Manufacturer
    "RP2350B HID Keyboard",     // 2: Product
    serial_str,                 // 3: Serial (chip unique board ID)
};

/* Must be called before tusb_init(), i.e. before the host can read descriptors. */
void usb_descriptors_init(void) {
    pico_get_unique_board_id_string(serial_str, sizeof(serial_str));
}

/* ------------------------------------------------------------------ */
/* Configuration Descriptor (assembled by TinyUSB's TUD_CONFIG_DESCRIPTOR macro) */
/* ------------------------------------------------------------------ */
uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN, 0x00, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In & Out address, size, interval
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(desc_hid_report), 0x81, CFG_TUD_HID_EP_BUFSIZE, 10),
};

/* ------------------------------------------------------------------ */
/* Callbacks: return the descriptor matching a config/string index     */
/* ------------------------------------------------------------------ */
uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

/* Return the HID report descriptor (required by TinyUSB) */
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    return desc_hid_report;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t _desc_str[32 + 1];
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
            return NULL;
        }
        const char *str = string_desc_arr[index];
        chr_count = (uint8_t)strlen(str);
        if (chr_count > 31) {
            chr_count = 31;
        }
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}
