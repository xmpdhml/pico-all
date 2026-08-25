#include "system.h"

#include <stdio.h>
#include <iostream>
#include <string>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/error.h"
#include "hardware/gpio.h"

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
    .pullup     = false,
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
      scan_instance(matrix_instance),
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

    std::cout << "Starting main loop with standard I/O..." << std::endl;
}

void System::run()
{
    std::cout << "Core0 is running..." << std::endl;

    // UART 控制台行缓冲（非阻塞读取）
    std::string console_line;
    bool led_state = false;

    while (true) {
        // 扫描 + 去抖 + TinyUSB/HID 后台任务（每轮都跑，保证 HID 及时响应）
        scan.scan();
        hid.task();

        // LED 闪烁指示主循环存活
        gpio_put(PIN_LED_STATUS, led_state);
        led_state = !led_state;

        // 非阻塞读取 UART 控制台输入（不阻塞主循环，保证 USB 及时响应）
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

        sleep_ms(10); // 10ms：USB HID 轮询周期
    }
}

