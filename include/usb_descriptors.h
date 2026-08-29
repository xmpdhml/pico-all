/*
 * usb_descriptors.h
 * USB 设备/配置/HID 报告描述符声明（TinyUSB HID 键盘，6KRO + NKRO 双报告）。
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "tusb.h"

/* 报告 ID：
 *   1 = 6KRO（Boot 键盘格式：modifier + 6 键 + LED 输出）
 *   2 = NKRO（256 键位图：modifier + 保留 + 32 字节位图）
 *   3 = Consumer 媒体（位图，多键同报）
 *   4 = Consumer 系统键 Power/Reset/Sleep（位图）
 *   5 = Consumer 应用启动 AL_*（位图） */
#define REPORT_ID_KEYBOARD       1
#define REPORT_ID_NKRO           2
#define REPORT_ID_CONSUMER       3
#define REPORT_ID_CONSUMER_SYS   4
#define REPORT_ID_CONSUMER_APP   5

/* NKRO 位图大小（256 键码 = 32 字节） */
#define NKRO_BITMAP_SIZE    32

/* Consumer 媒体控制位图：覆盖 usage 0xB0~0xEF（64 位 = 8 字节）。
 * 每个 bit 对应一个 usage（bit N = usage 0xB0+N），支持多键同报。
 * 覆盖：Play/Pause(0xCD)、Stop(0xB7)、Next/Prev(0xB5/0xB6)、
 *       Mute(0xE2)、Vol+/-(0xE9/0xEA) 等。 */
#define CONSUMER_USAGE_MIN   0xB0
#define CONSUMER_USAGE_MAX   0xEF
#define CONSUMER_BITMAP_SIZE 8   /* (0xEF - 0xB0 + 1) / 8 = 64 bit */

/* Consumer 系统键位图：usage 0x30~0x32（Power/Reset/Sleep），3 bit = 1 字节。
 * bit N = usage (CONSUMER_SYS_USAGE_MIN + N)。 */
#define CONSUMER_SYS_USAGE_MIN   0x30
#define CONSUMER_SYS_USAGE_MAX   0x32
#define CONSUMER_SYS_BITMAP_SIZE 1   /* (0x32 - 0x30 + 1) / 8 -> 1 字节 */

/* Consumer 应用启动（AL_*）位图：usage 0x184~0x1C6（67 bit = 9 字节）。
 * KEY_E_AL_WORD..KEY_E_AL_AUDIO_PLAYER 按枚举顺序偏移映射到该窗口，
 * 含 AL Power Status（0x1A7），覆盖全部 67 个 AL usage。 */
#define CONSUMER_APP_USAGE_MIN   0x184
#define CONSUMER_APP_USAGE_MAX   0x1C6
#define CONSUMER_APP_BITMAP_SIZE 9   /* (0x1C6 - 0x184 + 1) / 8 = 67 bit */

extern const tusb_desc_device_t desc_device;
extern const uint8_t desc_hid_report[];
extern const char *string_desc_arr[];

#endif /* USB_DESCRIPTORS_H */
