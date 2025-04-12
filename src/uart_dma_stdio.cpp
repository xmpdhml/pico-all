#include "uart_dma_stdio.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <memory>
#include <atomic>
#include <deque>

#include "hardware/irq.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

#include "mutex_wrap.h"

static const int IO_TX_QUEUE_SIZE = 16;

static Mutex dma_tx_mutex;
static std::deque<std::string> dma_tx_queue(IO_TX_QUEUE_SIZE);

static uart_inst_t* uart_instance = nullptr;

static void uart_dma_write(const char *buf, int len) {
    std::string str(buf, len);
    while (true) {
        while (dma_tx_queue.size() >= IO_TX_QUEUE_SIZE) {
            tight_loop_contents();
        }
        MUTEX_LOCK(dma_tx_mutex) {
            if (dma_tx_queue.size() < IO_TX_QUEUE_SIZE) {
                dma_tx_queue.push_back(str);
                break;
            }
        }
    }
}

static int uart_poll_read(char *buf, int len) {
    int count = 0;
    
    while (count < len && uart_is_readable(uart_instance)) {
        buf[count++] = uart_getc(uart_instance);
    }
    
    return count;
}

static stdio_driver_t uart_dma_stdio = {
    .out_chars = uart_dma_write,
    .out_flush = nullptr,
    .in_chars = uart_poll_read,
    .crlf_enabled = true
};

static void core1_uart_tx() {
    while (true) {
        while (dma_tx_queue.empty()) {
            tight_loop_contents();
        }
        
        std::string str;
        MUTEX_LOCK(dma_tx_mutex) {
            if (dma_tx_queue.empty()) {
                continue;
            }
            str = dma_tx_queue.front();
            dma_tx_queue.pop_front();
        }
        
        for (size_t i = 0; i < str.size(); i++) {
            uart_putc_raw(uart_instance, str[i]);
        }
    }
}

void UartDMAStdio::init(int pin_tx, int pin_rx, int baud_rate, uart_inst_t *uart, int dma) {
    uart_instance = uart ? uart : uart0;
    
    uart_init(uart_instance, baud_rate);
    gpio_set_function(pin_tx, GPIO_FUNC_UART);
    gpio_set_function(pin_rx, GPIO_FUNC_UART);
    
    multicore_launch_core1(core1_uart_tx);
    
    stdio_set_driver_enabled(&uart_dma_stdio, true);
}

void UartDMAStdio::deinit() {
    stdio_set_driver_enabled(&uart_dma_stdio, false);
    uart_deinit(uart_instance);
}

