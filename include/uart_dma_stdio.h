#pragma once

#include "hardware/uart.h"
#include "pico/stdio/driver.h"

/* Direct, blocking UART write that does NOT depend on FreeRTOS: no queue and no
 * task involved, so it also works before the scheduler starts and from
 * assert/fault paths.
 *
 * Use this for early boot milestones. The normal printf/DEBUG_LOG path is pumped
 * by the uart_tx FreeRTOS task, so if the scheduler never starts it produces no
 * output at all — which makes a startup failure completely silent. */
extern "C" void uart_boot_puts(const char* s);

class UartDMAStdio
{
    friend class System;

public:
    void init(int pin_tx, int pin_rx, int baud_rate, uart_inst_t *uart = nullptr, int dma = -1);
    void deinit();

private:
    UartDMAStdio() = default;
    ~UartDMAStdio() = default;
    UartDMAStdio(const UartDMAStdio&) = delete;
    UartDMAStdio& operator=(const UartDMAStdio&) = delete;
    UartDMAStdio(UartDMAStdio&&) = delete;
    UartDMAStdio& operator=(UartDMAStdio&&) = delete;

};