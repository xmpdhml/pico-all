#include <stdio.h>
#include <string.h>
#include <iostream>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "system.h"
#include "pinlist.h"


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

    // Set up LED pin
    gpio_init(PIN_LED_STATUS);
    gpio_set_dir(PIN_LED_STATUS, GPIO_OUT);

    sleep_ms(1000);
    
    std::cout << "Starting main loop with standard I/O..." << std::endl;

    // Main loop on core0
    bool led_state = false;

    // Set cin to non-blocking mode (note: this is platform-specific)
    std::cin.unsetf(std::ios::skipws); // Don't skip whitespace

    while (1) {
        std::cout << "Core0 is running..." << std::endl;
        std::cout << "- " << std::flush;
        gpio_put(PIN_LED_STATUS, led_state);
        led_state = !led_state;

        std::string input;
        std::getline(std::cin, input); // Read a line from standard input
        if (!input.empty()) {
            std::cout << "Received: " << input << std::endl;
        }
        
        sleep_ms(200); // Small delay to prevent busy waiting
    }

    return 0;
}
