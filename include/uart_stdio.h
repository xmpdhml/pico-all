// #pragma once

// #include "service.h"
// #include "hardware/uart.h"
// #include "pico/stdio/driver.h"

// struct UartStdioServiceCreationContext {
//     uart_inst_t *uart;
//     uint baudrate;
//     uint tx_pin;
//     uint rx_pin;
// };

// class UartStdioService : public Service {
// private:
//     UartStdioService(const UartStdioServiceCreationContext &context);
//     stdio_driver_t stdio_uart;
// public:
//     virtual ~UartStdioService();
//     static UartStdioService *create(const UartStdioServiceCreationContext &context);
// };

// template<>
// UartStdioService *Service::create<UartStdioService, UartStdioServiceCreationContext>(const UartStdioServiceCreationContext &context);
