#include "system.h"
#include "keyboard_config.h"

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

#include "debug_log.h"

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

/* 引脚与键位定义集中在 include/keyboard_config.h（keyboard_config 命名空间），此处只引用。 */
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
    // 自定义 UART DMA stdio（调试输出）——必须最先初始化，
    // 之后 DEBUG_LOG/printf 才会真正经 uart_dma_stdio 输出（此前无任何已启用驱动，输出被静默丢弃）
    io.init(cfg::kUartTxPin, cfg::kUartRxPin, BAUD_RATE);
    printf("Hello, world! customized\n");
    DEBUG_LOG("SYS", "init begin");

    // TinyUSB HID 键盘（上电枚举为 USB 键盘）
    hid.init();
    DEBUG_LOG("SYS", "USB HID init done");

    // 矩阵 GPIO（行输出 / 列输入 + 去抖）
    scan.init();
    DEBUG_LOG("SYS", "matrix scan init done (%dx%d)", matrix.rows(), matrix.cols());

    // LED 指示
    gpio_init(cfg::kLedPin);
    gpio_set_dir(cfg::kLedPin, GPIO_OUT);
    DEBUG_LOG("SYS", "LED init done (GP%d)", cfg::kLedPin);

    sleep_ms(1000);

    std::cout << "Starting FreeRTOS SMP scheduler..." << std::endl;
}

void System::run()
{
    // 创建任务（在调度器启动前创建；任务体不会在 vTaskStartScheduler 前运行）
    DEBUG_LOG("SYS", "creating tasks (keyboard prio4, system prio1)");
    xTaskCreate(&System::keyboard_task, "keyboard", 1024, nullptr, 4, nullptr);
    xTaskCreate(&System::system_task,   "system",   1024, nullptr, 1, nullptr);

    // 启动调度器：双核 SMP，永不返回（除非堆不足导致空闲任务创建失败）
    DEBUG_LOG("SYS", "starting FreeRTOS SMP scheduler on %d cores", configNUMBER_OF_CORES);
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
    DEBUG_LOG("SCAN", "keyboard task running on core %d", get_core_num());
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        // 扫描 + 去抖 + TinyUSB/HID 上报 + 内部动作（与旧主循环同节奏：10ms）
        sys.scan.scan();
        sys.hid.task();
        sys.handle_reset_actions();

        // 仅在有状态跳变时打印按键变化（按下/释放沿），避免 10ms 周期刷屏
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
        // LED 心跳：指示任务存活
        gpio_put(cfg::kLedPin, led_state);
        led_state = !led_state;

        // ---- 遗留的 UART echo 回显测试代码，已注释（保留以备日后参考） ----
        // std::string console_line;   // 控制台行缓冲（非阻塞读取）
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
            case KEY_P_USB_BURN:
                DEBUG_LOG("SYS", "reset action: USB_BURN -> UF2 bootloader");
                Reset(true);  break;   // 进引导
            case KEY_P_REBOOT:
                DEBUG_LOG("SYS", "reset action: REBOOT -> soft reset");
                Reset(false); break;   // 软复位
            default: break;
        }
    }
    prev_internal_ = scan.pressed_internal();
}

