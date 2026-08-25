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
 *   3 = Consumer（媒体控制位图，多键同报） */
#define REPORT_ID_KEYBOARD  1
#define REPORT_ID_NKRO      2
#define REPORT_ID_CONSUMER  3

/* NKRO 位图大小（256 键码 = 32 字节） */
#define NKRO_BITMAP_SIZE    32

/* Consumer 媒体控制位图：覆盖 usage 0xB0~0xEF（64 位 = 8 字节）。
 * 每个 bit 对应一个 usage（bit N = usage 0xB0+N），支持多键同报。
 * 覆盖：Play/Pause(0xCD)、Stop(0xB7)、Next/Prev(0xB5/0xB6)、
 *       Mute(0xE2)、Vol+/-(0xE9/0xEA) 等。 */
#define CONSUMER_USAGE_MIN   0xB0
#define CONSUMER_USAGE_MAX   0xEF
#define CONSUMER_BITMAP_SIZE 8   /* (0xEF - 0xB0 + 1) / 8 = 64 bit */

extern const tusb_desc_device_t desc_device;
extern const uint8_t desc_hid_report[];
extern const char *string_desc_arr[];

#endif /* USB_DESCRIPTORS_H */
