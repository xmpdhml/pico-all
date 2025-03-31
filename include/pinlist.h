#pragma once

#include "pico/types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum : uint
{
    PIN_UART_IO_OUT = 0,
    PIN_UART_IO_IN = 1,

    PIN_LED_STATUS = 2,

    PIN_ROW1 = 32,
    PIN_ROW2 = 34,
    PIN_ROW3 = 36,
    PIN_COL0 = 38,
    PIN_COL1 = 40,
    PIN_COL2 = 42,
};

#ifdef __cplusplus
}
#endif
