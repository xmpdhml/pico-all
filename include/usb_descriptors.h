/*
 * usb_descriptors.h
 * USB device/configuration/HID report descriptor declarations
 * (TinyUSB HID keyboard, dual report: 6KRO + NKRO).
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "tusb.h"

/* Report IDs:
 *   1 = 6KRO (Boot keyboard format: modifier + 6 keys + LED output)
 *   2 = NKRO (256-key bitmap: modifier + reserved + 32-byte bitmap)
 *   3 = Consumer media (bitmap, multiple keys per report)
 *   4 = Consumer system keys Power/Reset/Sleep (bitmap)
 *   5 = Consumer application launch AL_* (bitmap) */
#define REPORT_ID_KEYBOARD       1
#define REPORT_ID_NKRO           2
#define REPORT_ID_CONSUMER       3
#define REPORT_ID_CONSUMER_SYS   4
#define REPORT_ID_CONSUMER_APP   5

/* NKRO bitmap size (256 keycodes = 32 bytes) */
#define NKRO_BITMAP_SIZE    32

/* Consumer media bitmap: covers usage 0xB0~0xEF (64 bits = 8 bytes).
 * Each bit maps to one usage (bit N = usage 0xB0+N), multi-key per report.
 * Covers: Play/Pause(0xCD), Stop(0xB7), Next/Prev(0xB5/0xB6),
 *         Mute(0xE2), Vol+/-(0xE9/0xEA), etc. */
#define CONSUMER_USAGE_MIN   0xB0
#define CONSUMER_USAGE_MAX   0xEF
#define CONSUMER_BITMAP_SIZE 8   /* (0xEF - 0xB0 + 1) / 8 = 64 bit */

/* Consumer system keys bitmap: usage 0x30~0x32 (Power/Reset/Sleep), 3 bits = 1 byte.
 * bit N = usage (CONSUMER_SYS_USAGE_MIN + N). */
#define CONSUMER_SYS_USAGE_MIN   0x30
#define CONSUMER_SYS_USAGE_MAX   0x32
#define CONSUMER_SYS_BITMAP_SIZE 1   /* (0x32 - 0x30 + 1) / 8 -> 1 byte */

/* Consumer application launch (AL_*) bitmap: usage 0x184~0x1C6 (67 bits = 9 bytes).
 * KEY_E_AL_WORD..KEY_E_AL_AUDIO_PLAYER map into this window by enum order,
 * including AL Power Status (0x1A7), covering all 67 AL usages. */
#define CONSUMER_APP_USAGE_MIN   0x184
#define CONSUMER_APP_USAGE_MAX   0x1C6
#define CONSUMER_APP_BITMAP_SIZE 9   /* (0x1C6 - 0x184 + 1) / 8 = 67 bit */

extern const tusb_desc_device_t desc_device;
extern const uint8_t desc_hid_report[];
extern const char *string_desc_arr[];

#endif /* USB_DESCRIPTORS_H */
