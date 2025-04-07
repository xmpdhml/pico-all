#pragma once

#include "hardware/uart.h"
#include "pico/stdio/driver.h"

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

    static void thread_get_data();

};