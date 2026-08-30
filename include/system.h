#pragma once

#include <vector>

#include "uart_dma_stdio.h"
#include "key_matrix.h"
#include "gpio_matrix_io.h"
#include "key_scan.h"
#include "usb_hid.h"

class System
{
    friend System& internal_create();

public:
    UartDMAStdio& io;
    KeyMatrix& matrix;
    KeyScanner& scan;
    UsbHid& hid;

    void init();   // 一次性初始化：UART stdio / USB HID / 矩阵 / LED
    void run();    // 创建 FreeRTOS 任务并启动调度器，永不返回
    void Reset(bool to_bootloader = true);   // true=进 UF2 引导, false=软复位

private:
    System();
    ~System() = default;
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    System& operator=(System&&) = delete;

    void handle_reset_actions();   // 消费 KEY_P_USB_BURN / KEY_P_REBOOT（按下沿）

    // FreeRTOS 任务体（静态成员函数：xTaskCreate 需要 C 函数指针，且可访问私有成员）
    static void keyboard_task(void* pv);   // 10ms：扫描 + HID 上报 + 内部动作
    static void system_task(void* pv);     // 10ms：LED 心跳 + 控制台输入

    UartDMAStdio io_instance;
    KeyMatrix matrix_instance;
    GpioMatrixIO matrix_io_instance;
    KeyScanner scan_instance;
    UsbHid hid_instance;
    std::vector<KeyCodes> prev_internal_;   // 复位键按下沿检测
};

extern System& sys;
