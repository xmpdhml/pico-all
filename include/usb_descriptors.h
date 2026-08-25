/*
 * usb_descriptors.h
 * USB 设备/配置/HID 报告描述符声明（TinyUSB HID 键盘，6KRO + NKRO 双报告）。
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "tusb.h"

/* 报告 ID：
 *   1 = 6KRO（Boot 键盘格式：modifier + 6 键 + LED 输出）
 *   2 = NKRO（256 键位图：modifier + 保留 + 32 字节位图） */
#define REPORT_ID_KEYBOARD  1
#define REPORT_ID_NKRO      2

/* NKRO 位图大小（256 键码 = 32 字节） */
#define NKRO_BITMAP_SIZE    32

extern const tusb_desc_device_t desc_device;
extern const uint8_t desc_hid_report[];
extern const char *string_desc_arr[];

#endif /* USB_DESCRIPTORS_H */
