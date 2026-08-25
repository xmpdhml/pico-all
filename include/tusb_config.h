/*
 * tusb_config.h
 * TinyUSB 配置：本项目仅启用 HID 设备类（键盘）。
 *
 * 注意：CFG_TUSB_MCU 由 Pico SDK 的 tinyusb 集成自动提供。RP2040 与 RP2350
 *       共用同一套 TinyUSB rp2040 驱动（OPT_MCU_RP2040），因此这里默认值对
 *       RP2350B 同样成立；用 #ifndef 保护以免重复定义。
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- 板级/RHPort 配置 ------------------- */
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE)

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU            OPT_MCU_RP2040
#endif

/* ------------------- 通用配置 ------------------- */
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS             OPT_OS_NONE   /* 无 RTOS，裸机轮询 */
#endif
#define CFG_TUSB_DEBUG          0

#define CFG_TUD_ENABLED         1

/* 传输缓冲区内存属性（RP2040/RP2350 由 SDK 处理，留空即可） */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif
#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN      __attribute__((aligned(4)))
#endif

/* ------------------- 设备类驱动开关 ------------------- */
#define CFG_TUD_CDC             0
#define CFG_TUD_MSC             0
#define CFG_TUD_HID             1   /* 启用 HID 键盘 */
#define CFG_TUD_MIDI            0
#define CFG_TUD_VENDOR          0

/* HID 端点传输缓冲区大小 */
#ifndef CFG_TUD_HID_EP_BUFSIZE
#define CFG_TUD_HID_EP_BUFSIZE  64
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
