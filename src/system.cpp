#include "system.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/error.h"
#include "pico/bootrom.h"       // reset_usb_boot：软件进 UF2 引导
#include "hardware/gpio.h"
#include "hardware/watchdog.h"  // watchdog_reboot：软复位

#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS 栈溢出钩子：致命错误，立即停机。
 * 溢出时栈可能已耗尽，不做 printf 等复杂操作，直接死循环。 */
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

/* 3×3 矩阵布局（引脚取自 pinlist.h；键码表为占位，按实际 PCB 调整） */
static const uint8_t kRowPins[] = { PIN_ROW1, PIN_ROW2, PIN_ROW3 };
static const uint8_t kColPins[] = { PIN_COL0, PIN_COL1, PIN_COL2 };
static const KeyCodes kKeymap[3][3] = {
    { KEY_A, KEY_B, KEY_C },
    { KEY_D, KEY_E, KEY_F },
    { KEY_G, KEY_H, KEY_I },
};
static const KeyMatrix::Config kMatrixConfig = {
    .rows       = 3,
    .cols       = 3,
    .row_pins   = kRowPins,
    .col_pins   = kColPins,
    .keymap     = &kKeymap[0][0],
    .direction  = KeyMatrix::Direction::RowToCol,
};

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
      matrix_instance(kMatrixConfig),
      matrix_io_instance(matrix_instance),
      scan_instance(matrix_instance, matrix_io_instance),
      hid_instance(scan_instance)
{
}

void System::init()
{
    // 自定义 UART DMA stdio（调试输出）
    io.init(PIN_UART_IO_OUT, PIN_UART_IO_IN, BAUD_RATE);
    printf("Hello, world! customized\n");

    // TinyUSB HID 键盘（上电枚举为 USB 键盘）
    hid.init();

    // 矩阵 GPIO（行输出 / 列输入 + 去抖）
    scan.init();

    // LED 指示
    gpio_init(PIN_LED_STATUS);
    gpio_set_dir(PIN_LED_STATUS, GPIO_OUT);

    sleep_ms(1000);

    std::cout << "Starting FreeRTOS SMP scheduler..." << std::endl;
}

void System::run()
{
    // 创建任务（在调度器启动前创建；任务体不会在 vTaskStartScheduler 前运行）
    xTaskCreate(&System::keyboard_task, "keyboard", 1024, nullptr, 4, nullptr);
    xTaskCreate(&System::system_task,   "system",   1024, nullptr, 1, nullptr);

    // 启动调度器：双核 SMP，永不返回（除非堆不足导致空闲任务创建失败）
    vTaskStartScheduler();

    // 调度器启动失败：报告并停机
    std::cout << "FATAL: vTaskStartScheduler() returned" << std::endl;
    while (true) {
        tight_loop_contents();
    }
}

void System::keyboard_task(void* pv)
{
    (void)pv;
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        // 扫描 + 去抖 + TinyUSB/HID 上报 + 内部动作（与旧主循环同节奏：10ms）
        sys.scan.scan();
        sys.hid.task();
        sys.handle_reset_actions();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}

void System::system_task(void* pv)
{
    (void)pv;
    std::string console_line;   // 控制台行缓冲（非阻塞读取）
    bool led_state = false;
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        // LED 心跳：指示任务存活
        gpio_put(PIN_LED_STATUS, led_state);
        led_state = !led_state;

        // 非阻塞读取 UART 控制台输入
        int c;
        while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                if (!console_line.empty()) {
                    std::cout << "Received: " << console_line << std::endl;
                    console_line.clear();
                }
            } else if (c >= 32 && c < 127) {
                console_line += (char)c;
            }
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}

void System::Reset(bool to_bootloader)
{
    if (to_bootloader) {
        // 进入 UF2 引导（U 盘模式），不返回
        reset_usb_boot(0, 0);
    } else {
        // 软复位：重新执行固件
        watchdog_reboot(0, 0, 0);
    }
}

void System::handle_reset_actions()
{
    for (KeyCodes k : scan.pressed_internal()) {
        if (std::find(prev_internal_.begin(), prev_internal_.end(), k)
                != prev_internal_.end()) {
            continue; // 之前已按下，不是上升沿
        }
        switch (k) {
            case KEY_P_USB_BURN: Reset(true);  break;   // 进引导
            case KEY_P_REBOOT:   Reset(false); break;   // 软复位
            default: break;
        }
    }
    prev_internal_ = scan.pressed_internal();
}

