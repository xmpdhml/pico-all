/*
 * usb_descriptors.c
 * RP2350B USB HID 键盘的 TinyUSB 描述符定义（6KRO + NKRO 双报告）。
 *
 * 参考：Pico SDK 示例 / lib/tinyusb/examples/device/hid_composite
 */

#include <string.h>

#include "tusb.h"
#include "usb_descriptors.h"

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
/* HID Report Descriptor（6KRO + NKRO 双报告）                          */
/*   Report ID 1 = 6KRO：Boot 键盘格式（修饰键 + 6 键 + LED 输出）       */
/*   Report ID 2 = NKRO：256 键位图（修饰键 + 保留 + 32 字节位图）       */
/* 固件按当前模式只发送其中一个报告（见 usb_hid.cpp 的 nkro_mode）。     */
/* ------------------------------------------------------------------ */
uint8_t const desc_hid_report[] = {
    /* ================= 6KRO（Boot 键盘格式） ================= */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ),
    HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_KEYBOARD )
      /* 8 位修饰键（Ctrl/Shift/Alt/Gui） */
      HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ),
        HID_USAGE_MIN    ( 224 ),
        HID_USAGE_MAX    ( 231 ),
        HID_LOGICAL_MIN  ( 0 ),
        HID_LOGICAL_MAX  ( 1 ),
        HID_REPORT_COUNT ( 8 ),
        HID_REPORT_SIZE  ( 1 ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
        /* 8 位保留 */
        HID_REPORT_COUNT ( 1 ),
        HID_REPORT_SIZE  ( 8 ),
        HID_INPUT        ( HID_CONSTANT ),
      /* 输出 5 位 LED（Num/Caps/Scroll/Kana/Compose） */
      HID_USAGE_PAGE ( HID_USAGE_PAGE_LED ),
        HID_USAGE_MIN    ( 1 ),
        HID_USAGE_MAX    ( 5 ),
        HID_REPORT_COUNT ( 5 ),
        HID_REPORT_SIZE  ( 1 ),
        HID_OUTPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
        HID_REPORT_COUNT ( 1 ),
        HID_REPORT_SIZE  ( 3 ),
        HID_OUTPUT       ( HID_CONSTANT ),
      /* 6 字节键码 */
      HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ),
        HID_USAGE_MIN    ( 0 ),
        HID_USAGE_MAX_N  ( 255, 2 ),
        HID_LOGICAL_MIN  ( 0 ),
        HID_LOGICAL_MAX_N( 255, 2 ),
        HID_REPORT_COUNT ( 6 ),
        HID_REPORT_SIZE  ( 8 ),
        HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ),
    HID_COLLECTION_END,

    /* ================= NKRO（256 键位图） ================= */
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
        /* 256 位键码位图（32 字节） */
        HID_USAGE_MIN    ( 0 ),
        HID_USAGE_MAX_N  ( 255, 2 ),
        HID_LOGICAL_MIN  ( 0 ),
        HID_LOGICAL_MAX  ( 1 ),
        HID_REPORT_COUNT_N ( 256, 2 ),
        HID_REPORT_SIZE  ( 1 ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
    HID_COLLECTION_END,

    /* ============ Consumer / 媒体控制（位图，多键同报） ============ */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ),
    HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_CONSUMER )
      /* 64 位位图：bit N = usage (CONSUMER_USAGE_MIN + N)，1 bit/键 */
      HID_LOGICAL_MIN ( 0 ),
      HID_LOGICAL_MAX ( 1 ),
      HID_USAGE_MIN   ( CONSUMER_USAGE_MIN ),
      HID_USAGE_MAX   ( CONSUMER_USAGE_MAX ),
      HID_REPORT_COUNT( (CONSUMER_USAGE_MAX - CONSUMER_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
    HID_COLLECTION_END,

    /* ============ Consumer / 系统键 Power/Reset/Sleep（位图） ============ */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ),
    HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_CONSUMER_SYS )
      /* 3 位位图：bit N = usage (CONSUMER_SYS_USAGE_MIN + N) */
      HID_LOGICAL_MIN ( 0 ),
      HID_LOGICAL_MAX ( 1 ),
      HID_USAGE_MIN   ( CONSUMER_SYS_USAGE_MIN ),
      HID_USAGE_MAX   ( CONSUMER_SYS_USAGE_MAX ),
      HID_REPORT_COUNT( (CONSUMER_SYS_USAGE_MAX - CONSUMER_SYS_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
    HID_COLLECTION_END,

    /* ============ Consumer / 应用启动 AL_*（位图） ============ */
    HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ),
    HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
      HID_REPORT_ID ( REPORT_ID_CONSUMER_APP )
      /* 67 位位图：bit N = usage (CONSUMER_APP_USAGE_MIN + N)（usage>255 用 2 字节） */
      HID_LOGICAL_MIN ( 0 ),
      HID_LOGICAL_MAX ( 1 ),
      HID_USAGE_MIN_N ( CONSUMER_APP_USAGE_MIN, 2 ),
      HID_USAGE_MAX_N ( CONSUMER_APP_USAGE_MAX, 2 ),
      HID_REPORT_COUNT( (CONSUMER_APP_USAGE_MAX - CONSUMER_APP_USAGE_MIN + 1) ),
      HID_REPORT_SIZE ( 1 ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
    HID_COLLECTION_END,
};

/* ------------------------------------------------------------------ */
/* String Descriptors                                                  */
/* ------------------------------------------------------------------ */
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: Language ID = English
    "MyKeyboard",               // 1: Manufacturer
    "RP2350B HID Keyboard",    // 2: Product
    "000000",                   // 3: Serials
};

/* ------------------------------------------------------------------ */
/* Configuration Descriptor（由 TinyUSB 的 TUD_CONFIG_DESCRIPTOR 宏组合）*/
/* ------------------------------------------------------------------ */
uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN, 0x00, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In & Out address, size, interval
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(desc_hid_report), 0x81, CFG_TUD_HID_EP_BUFSIZE, 10),
};

/* ------------------------------------------------------------------ */
/* 回调：根据配置/字符串索引返回对应描述符                              */
/* ------------------------------------------------------------------ */
uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

/* 返回 HID 报告描述符（TinyUSB 必须的回调） */
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
