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

    void init();   // One-time init: UART stdio / USB HID / matrix / LED
    void run();    // Create FreeRTOS tasks and start the scheduler; never returns
    void Reset(bool to_bootloader = true);   // true=enter UF2 bootloader, false=soft reset

private:
    System();
    ~System() = default;
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    System& operator=(System&&) = delete;

    void handle_reset_actions();   // Consume KEY_P_USB_BURN / KEY_P_REBOOT (press edge)

    // FreeRTOS task bodies (static members: xTaskCreate needs a C function
    // pointer, and these need access to private members)
    static void keyboard_task(void* pv);   // 10ms: scan + HID report + internal actions
    static void system_task(void* pv);     // 10ms: LED heartbeat + console input

    UartDMAStdio io_instance;
    KeyMatrix matrix_instance;
    GpioMatrixIO matrix_io_instance;
    KeyScanner scan_instance;
    UsbHid hid_instance;
    std::vector<KeyCodes> prev_internal_;   // Reset-key press-edge detection
};

extern System& sys;
