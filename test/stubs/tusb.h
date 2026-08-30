/*
 * tusb.h — minimal TinyUSB stand-in for host unit tests.
 *
 * Provides only the types and function declarations needed to compile/link
 * usb_hid; the actual "send recording" lives in tusb_stub.cpp (test observer).
 */
#ifndef TUSB_H
#define TUSB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Minimal types ---- */
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

/* ---- Device / HID API (stub) ---- */
void tusb_init(void);
void tud_task(void);
bool tud_hid_ready(void);
bool tud_hid_report(uint8_t report_id, void const* report, uint16_t len);
bool tud_hid_keyboard_report(uint8_t report_id, uint8_t modifier, uint8_t keycode[6]);

#ifdef __cplusplus
}
#endif

#endif /* TUSB_H */
