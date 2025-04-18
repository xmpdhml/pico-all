#pragma once

#include "pinlist.h"

#include "uart_dma_stdio.h"
#include "key_scan.h"

class System
{
    friend System& internal_create();

public:
    UartDMAStdio& io;

    void Reset(bool to_bootloader = true);

private:
    System();
    ~System() = default;
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    System& operator=(System&&) = delete;

    UartDMAStdio io_instance;
};

extern System& sys;
