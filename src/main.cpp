#include <stdio.h>
#include <string.h>
#include <iostream>

#include "pico/stdlib.h"
#include "pico/stdio/driver.h"
#include "hardware/uart.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include <atomic>

// Define UART constants
#ifdef PICO_DEFAULT_UART
#   if PICO_DEFAULT_UART == 0
#       define UART_ID uart0
#   elif PICO_DEFAULT_UART == 1
#       define UART_ID uart1
#   else
#       error Invalid PICO_DEFAULT_UART value
#   endif
#else
#   define UART_ID uart0
#endif
#ifdef PICO_DEFAULT_UART_TX_PIN
#   define UART_TX_PIN PICO_DEFAULT_UART_TX_PIN
#else
#   define UART_TX_PIN 0
#endif
#ifdef PICO_DEFAULT_UART_RX_PIN
#   define UART_RX_PIN PICO_DEFAULT_UART_RX_PIN
#else
#   define UART_RX_PIN 1
#endif
#ifdef PICO_DEFAULT_UART_BAUD_RATE
#   define BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
#else
#   define BAUD_RATE 1500000
#endif

#define LED_PIN 14

// DMA channel for UART TX
int dma_tx_channel;
// Buffer for DMA transfers
char dma_tx_buffer[256];
//volatile std::atomic_bool dma_transfer_complete = true;
volatile bool dma_transfer_complete = true;

// DMA handler for core1
void dma_handler() {
    // Clear the interrupt request
    dma_hw->ints0 = 1u << dma_tx_channel;
    dma_transfer_complete = true;
}

// Function to configure DMA interrupt on core1
void core1_setup() {
    // Set up the DMA interrupt handler on core1
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Enable DMA channel interrupt
    dma_channel_set_irq0_enabled(dma_tx_channel, true);

    // Core1 main loop (optional)
    while (true) {
        tight_loop_contents();
    }
}

// Custom write function for stdio
void uart_dma_write(const char *buf, int len) {
    // Wait until previous transfer is complete
    while (!dma_transfer_complete) {
        tight_loop_contents();
    }
    
    // Don't transfer more than the buffer can hold
    if (len > (int)sizeof(dma_tx_buffer) - 1)
        len = (int)sizeof(dma_tx_buffer) - 1;
    
    // Copy data to the DMA buffer
    memcpy(dma_tx_buffer, buf, len);
    dma_tx_buffer[len] = '$'; // Add a delimiter for debugging
    len++;
    dma_transfer_complete = false;
    
    // Start DMA transfer
    dma_channel_set_read_addr(dma_tx_channel, dma_tx_buffer, false);
    dma_channel_set_trans_count(dma_tx_channel, len, true);
}

// Custom stdio descriptor
static stdio_driver_t uart_dma_stdio = {
    .out_chars = uart_dma_write,
    .out_flush = nullptr,
    .in_chars = nullptr,
    .crlf_enabled = true
};

int main() {

    stdio_init_all();

    printf("Hello, world! stdio\n");

    stdio_deinit_all();

    // Initialize UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Set up DMA channel for UART TX
    dma_tx_channel = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_tx_channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, uart_get_dreq(UART_ID, true));
    
    dma_channel_configure(
        dma_tx_channel,
        &c,
        &uart0_hw->dr,    // Write to UART data register for uart0
        NULL,            // Source address will be set later
        0,               // Transfer count will be set later
        false            // Don't start yet
    );
    
    // Launch core1 setup
    multicore_launch_core1(core1_setup);

    // Register our custom stdio driver
    stdio_set_driver_enabled(&uart_dma_stdio, true);
    
    printf("Hello, world! customized\n");

    // Set up LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Main loop on core0
    bool led_state = false;
    while (1) {
        sleep_ms(1000);
        printf("Core0 is running...\n");
        std::cout << "-" << std::flush;
        gpio_put(LED_PIN, led_state);
        led_state = !led_state;
    }

    // auto* uart_stdio = Service::create<UartStdioService>(
    //     UartStdioServiceCreationContext{
    //     .uart = UART_ID,
    //     .baudrate = BAUD_RATE,
    //     .tx_pin = UART_TX_PIN,
    //     .rx_pin = UART_RX_PIN
    // });
    
    return 0;
}
