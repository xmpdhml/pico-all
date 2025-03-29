#pragma once

#include "hardware/uart.h"
#include "pico/stdio/driver.h"

class UartDMAStdio
{
    friend class System;

public:
    void init();
    void deinit();

private:
    UartDMAStdio() = default;
    ~UartDMAStdio() = default;
    UartDMAStdio(const UartDMAStdio&) = delete;
    UartDMAStdio& operator=(const UartDMAStdio&) = delete;
    UartDMAStdio(UartDMAStdio&&) = delete;
    UartDMAStdio& operator=(UartDMAStdio&&) = delete;

};