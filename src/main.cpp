#include <iostream>

// #define PICO_DEFAULT_UART 0
// #define PICO_DEFAULT_UART_TX_PIN 0
// #define PICO_DEFAULT_UART_RX_PIN 1


#include "pico/stdlib.h"

int main() {

    // uart_init(uart0, 115200);
    // gpio_set_function(0, GPIO_FUNC_UART); // TX
    // gpio_set_function(1, GPIO_FUNC_UART); // RX
    
    stdio_init_all();
    
    std::cout << "Hello, world!" << std::endl;

    while (1) {
        sleep_ms(1000);
        std::cout << "." << std::flush;
    }
    
    return 0;
}
