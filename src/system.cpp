#include "system.h"
#include "keyboard_config.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/error.h"
#include "pico/bootrom.h"       // reset_usb_boot: enter UF2 bootloader via software
#include "hardware/gpio.h"
#include "hardware/watchdog.h"  // watchdog_reboot: soft reset

#include "debug_log.h"

#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS stack overflow hook: fatal error, halt immediately.
 * The stack may be exhausted on overflow, so no printf or other complex ops —
 * just loop forever. */
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    for (;;) {
        tight_loop_contents();
    }
}

#ifdef PICO_DEFAULT_UART_BAUD_RATE
#   define BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
#else
#   define BAUD_RATE 1500000
#endif

/* Pins and keymap are centralized in include/keyboard_config.h (keyboard_config namespace); referenced here only. */
namespace cfg = keyboard_config;

System& internal_create()
{
    static System instance;
    return instance;
}

System& sys = internal_create();

System::System()
    : io(io_instance),
      matrix(matrix_instance),
      scan(scan_instance),
      hid(hid_instance),
      matrix_instance(cfg::kMatrixConfig),
      matrix_io_instance(matrix_instance),
      scan_instance(matrix_instance, matrix_io_instance),
      hid_instance(scan_instance)
{
}

void System::init()
{
    // Custom UART DMA stdio (debug output) — must be initialized first, only
    // after which DEBUG_LOG/printf actually reach uart_dma_stdio (before that
    // no driver is enabled and output is silently discarded)
    io.init(cfg::kUartTxPin, cfg::kUartRxPin, BAUD_RATE);
    printf("Hello, world! customized\n");
    DEBUG_LOG("SYS", "init begin");

    // TinyUSB HID keyboard (enumerates as a USB keyboard at power-on)
    hid.init();
    DEBUG_LOG("SYS", "USB HID init done");

    // Matrix GPIO (rows out / columns in + debounce)
    scan.init();
    DEBUG_LOG("SYS", "matrix scan init done (%dx%d)", matrix.rows(), matrix.cols());

    // Status LED
    gpio_init(cfg::kLedPin);
    gpio_set_dir(cfg::kLedPin, GPIO_OUT);
    DEBUG_LOG("SYS", "LED init done (GP%d)", cfg::kLedPin);

    sleep_ms(1000);

    std::cout << "Starting FreeRTOS SMP scheduler..." << std::endl;
}

void System::run()
{
    // Create tasks (created before the scheduler starts; bodies don't run until vTaskStartScheduler)
    DEBUG_LOG("SYS", "creating tasks (keyboard prio4, system prio1)");
    xTaskCreate(&System::keyboard_task, "keyboard", 1024, nullptr, 4, nullptr);
    xTaskCreate(&System::system_task,   "system",   1024, nullptr, 1, nullptr);

    // Start the scheduler: dual-core SMP, never returns (unless the heap is too small to create the idle tasks)
    DEBUG_LOG("SYS", "starting FreeRTOS SMP scheduler on %d cores", configNUMBER_OF_CORES);
    vTaskStartScheduler();

    // Scheduler failed to start: report and halt
    // Scheduler failed to start: report and halt
    std::cout << "FATAL: vTaskStartScheduler() returned" << std::endl;
    while (true) {
        tight_loop_contents();
    }
}

void System::keyboard_task(void* pv)
{
    (void)pv;
    DEBUG_LOG("SCAN", "keyboard task running on core %d", get_core_num());
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        // Scan + debounce + TinyUSB/HID report + internal actions (same cadence as the old main loop: 10ms)
        sys.scan.scan();
        sys.hid.task();
        sys.handle_reset_actions();

        // Only log key changes on state transitions (press/release edges) to avoid spamming every 10ms
        if (sys.scan.basic_changed() || sys.scan.extended_changed() || sys.scan.internal_changed()) {
            DEBUG_LOG("SCAN", "keys changed: basic=%u ext=%u internal=%u",
                      (unsigned)sys.scan.pressed_basic().size(),
                      (unsigned)sys.scan.pressed_extended().size(),
                      (unsigned)sys.scan.pressed_internal().size());
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}

void System::system_task(void* pv)
{
    (void)pv;
    DEBUG_LOG("SYS", "system task running on core %d", get_core_num());
    bool led_state = false;
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        // LED heartbeat: indicates the task is alive
        gpio_put(cfg::kLedPin, led_state);
        led_state = !led_state;

        // ---- Legacy UART echo-back test code, commented out (kept for reference) ----
        // std::string console_line;   // Console line buffer (non-blocking read)
        // int c;
        // while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        //     if (c == '\r' || c == '\n') {
        //         if (!console_line.empty()) {
        //             std::cout << "Received: " << console_line << std::endl;
        //             console_line.clear();
        //         }
        //     } else if (c >= 32 && c < 127) {
        //         console_line += (char)c;
        //     }
        // }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}

void System::Reset(bool to_bootloader)
{
    if (to_bootloader) {
        // Enter UF2 bootloader (USB drive mode), does not return
        reset_usb_boot(0, 0);
    } else {
        // Soft reset: re-run the firmware
        watchdog_reboot(0, 0, 0);
    }
}

void System::handle_reset_actions()
{
    for (KeyCodes k : scan.pressed_internal()) {
        if (std::find(prev_internal_.begin(), prev_internal_.end(), k)
                != prev_internal_.end()) {
            continue; // Already pressed, not a rising edge
        }
        switch (k) {
            case KEY_P_USB_BURN:
                DEBUG_LOG("SYS", "reset action: USB_BURN -> UF2 bootloader");
                Reset(true);  break;   // Enter bootloader
            case KEY_P_REBOOT:
                DEBUG_LOG("SYS", "reset action: REBOOT -> soft reset");
                Reset(false); break;   // Soft reset
            default: break;
        }
    }
    prev_internal_ = scan.pressed_internal();
}

