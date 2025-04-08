#include "uart_dma_stdio.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

// Static member variables for the DMA channels and buffers
static int dma_tx_channel;
static char dma_tx_buffer[256];
static volatile bool dma_transfer_complete = true;

// Use different interrupt numbers for RX and TX
#define DMA_IRQ_TX DMA_IRQ_1

// UART instance to use
static uart_inst_t* uart_instance = nullptr;

// DMA handler for TX
static void dma_tx_handler() {
    // Clear the interrupt request for IRQ1
    dma_hw->ints1 = 1u << dma_tx_channel;
    dma_transfer_complete = true;
}

// Custom write function for stdio
static void uart_dma_write(const char *buf, int len) {
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

// Simple polling read function for stdio
static int uart_poll_read(char *buf, int len) {
    int count = 0;
    
    // Check if any characters available
    while (count < len && uart_is_readable(uart_instance)) {
        buf[count++] = uart_getc(uart_instance);
    }
    
    return count;
}

// Update the stdio driver
static stdio_driver_t uart_dma_stdio = {
    .out_chars = uart_dma_write,
    .out_flush = nullptr,
    .in_chars = uart_poll_read,
    .crlf_enabled = true
};

// Core1 setup function for handling DMA TX interrupts
static void core1_dma_setup() {
    // Set up the DMA interrupt handler on core1 for TX
    irq_set_exclusive_handler(DMA_IRQ_TX, dma_tx_handler);
    irq_set_enabled(DMA_IRQ_TX, true);

    // Enable DMA channel interrupt (use IRQ1 for TX)
    dma_channel_set_irq1_enabled(dma_tx_channel, true);

    // Core1 main loop (optional)
    while (true) {
        tight_loop_contents();
    }
}

void UartDMAStdio::init(int pin_tx, int pin_rx, int baud_rate, uart_inst_t *uart, int dma) {
    // Store UART instance
    uart_instance = uart ? uart : uart0;
    
    // Initialize UART
    uart_init(uart_instance, baud_rate);
    gpio_set_function(pin_tx, GPIO_FUNC_UART);
    gpio_set_function(pin_rx, GPIO_FUNC_UART);
    
    // Set up DMA channel for UART TX
    dma_tx_channel = (dma >= 0) ? dma : dma_claim_unused_channel(true);
    dma_channel_config tx_config = dma_channel_get_default_config(dma_tx_channel);
    channel_config_set_transfer_data_size(&tx_config, DMA_SIZE_8);
    channel_config_set_read_increment(&tx_config, true);
    channel_config_set_write_increment(&tx_config, false);
    channel_config_set_dreq(&tx_config, uart_get_dreq(uart_instance, true));
    
    dma_channel_configure(
        dma_tx_channel,
        &tx_config,
        &uart_get_hw(uart_instance)->dr,  // Write to UART data register
        NULL,                             // Source address will be set later
        0,                                // Transfer count will be set later
        false                             // Don't start yet
    );
    
    // Launch core1 to handle the TX DMA interrupts
    multicore_launch_core1(core1_dma_setup);
    
    // Register our custom stdio driver
    stdio_set_driver_enabled(&uart_dma_stdio, true);
}

// Also add the missing deinit() implementation
void UartDMAStdio::deinit() {
    // Disable our custom stdio driver
    stdio_set_driver_enabled(&uart_dma_stdio, false);
    
    // Abort any ongoing DMA transfers
    dma_channel_abort(dma_tx_channel);
    
    // Disable DMA interrupts
    irq_set_enabled(DMA_IRQ_TX, false);
    dma_channel_set_irq1_enabled(dma_tx_channel, false);
    
    // Release DMA channel
    dma_channel_unclaim(dma_tx_channel);
    
    // Deinitialize UART
    uart_deinit(uart_instance);
}

