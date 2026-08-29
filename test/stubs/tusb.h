/*
 * tusb.h —— 宿主机单元测试用的 TinyUSB 最小化替身。
 *
 * 只提供 usb_hid 编译/链接所需的类型与函数声明，
 * 具体"发送记录"在 tusb_stub.cpp 中实现（测试观察点）。
 */
#ifndef TUSB_H
#define TUSB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 最小化类型 ---- */
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} tusb_desc_device_t;

typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} hid_keyboard_report_t;

typedef enum {
    HID_REPORT_TYPE_INVALID = 0,
    HID_REPORT_TYPE_INPUT,
    HID_REPORT_TYPE_OUTPUT,
    HID_REPORT_TYPE_FEATURE,
} hid_report_type_t;

/* ---- 设备 / HID API（stub） ---- */
void tusb_init(void);
void tud_task(void);
bool tud_hid_ready(void);
bool tud_hid_report(uint8_t report_id, void const* report, uint16_t len);
bool tud_hid_keyboard_report(uint8_t report_id, uint8_t modifier, uint8_t keycode[6]);

#ifdef __cplusplus
}
#endif

#endif /* TUSB_H */
