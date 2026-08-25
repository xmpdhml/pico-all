#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/error.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "system.h"
#include "pinlist.h"
#include "usb_hid.h"


#ifdef PICO_DEFAULT_UART_BAUD_RATE
#   define BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
#else
#   define BAUD_RATE 1500000
#endif

int main() {
    stdio_init_all();
    printf("Hello, world! stdio\n");
    stdio_deinit_all();

    // Initialize our custom UART DMA stdio
    sys.io.init(PIN_UART_IO_OUT, PIN_UART_IO_IN, BAUD_RATE);
    
    printf("Hello, world! customized\n");

    // Initialize TinyUSB HID keyboard（上电枚举为 USB 键盘）
    usb_hid_init();

    // Set up LED pin
    gpio_init(PIN_LED_STATUS);
    gpio_set_dir(PIN_LED_STATUS, GPIO_OUT);

    sleep_ms(1000);
    
    std::cout << "Starting main loop with standard I/O..." << std::endl;

    // Main loop on core0
    bool led_state = false;

    std::cout << "Core0 is running..." << std::endl;

    // UART 控制台行缓冲（非阻塞读取）
    std::string console_line;

    while (1) {
        // 扫描按键 + TinyUSB/HID 后台任务（每轮都跑，保证 HID 及时响应）
        ks_scan();
        usb_hid_task();

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

        sleep_ms(10); // 10ms：USB HID 轮询周期（原 200ms 会拖慢 USB 响应）
    }

    return 0;
}
