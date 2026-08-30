/*
 * tusb_config.h
 * TinyUSB configuration: this project enables only the HID device class (keyboard).
 *
 * Note: CFG_TUSB_MCU is auto-provided by the Pico SDK's TinyUSB integration.
 *       RP2040 and RP2350 share the same TinyUSB rp2040 driver (OPT_MCU_RP2040),
 *       so the defaults below also hold for the RP2350B; guarded with #ifndef
 *       to avoid redefinition.
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- Board / RHPort config ------------------- */
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE)

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU            OPT_MCU_RP2040
#endif

/* ------------------- Common config ------------------- */
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS             OPT_OS_NONE   /* No RTOS, bare-metal polling */
#endif
#define CFG_TUSB_DEBUG          0

#define CFG_TUD_ENABLED         1

/* Transfer buffer memory attributes (handled by SDK on RP2040/RP2350; leave empty) */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif
#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN      __attribute__((aligned(4)))
#endif

/* ------------------- Device class driver switches ------------------- */
#define CFG_TUD_CDC             0
#define CFG_TUD_MSC             0
#define CFG_TUD_HID             1   /* Enable HID keyboard */
#define CFG_TUD_MIDI            0
#define CFG_TUD_VENDOR          0

/* HID endpoint transfer buffer size */
#ifndef CFG_TUD_HID_EP_BUFSIZE
#define CFG_TUD_HID_EP_BUFSIZE  64
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
