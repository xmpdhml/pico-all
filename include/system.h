#pragma once

#include <vector>

#include "pinlist.h"

#include "uart_dma_stdio.h"
#include "key_matrix.h"
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
    void run();    // 主循环：扫描 + HID 上报 + 控制台，永不返回
    void Reset(bool to_bootloader = true);   // true=进 UF2 引导, false=软复位

private:
    System();
    ~System() = default;
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    System& operator=(System&&) = delete;

    void handle_reset_actions();   // 消费 KEY_P_USB_BURN / KEY_P_REBOOT（按下沿）

    UartDMAStdio io_instance;
    KeyMatrix matrix_instance;
    KeyScanner scan_instance;
    UsbHid hid_instance;
    std::vector<KeyCodes> prev_internal_;   // 复位键按下沿检测
};

extern System& sys;
