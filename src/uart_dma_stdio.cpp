#include "uart_dma_stdio.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <memory>
#include <atomic>
#include <deque>
#include <utility>

#include "hardware/irq.h"
#include "hardware/gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#include "mutex_wrap.h"
#include "debug_log.h"

static const int IO_TX_QUEUE_SIZE = 16;

static Mutex dma_tx_mutex;
static std::deque<std::string> dma_tx_queue;   // 修复：不再预填 16 个空串
static TaskHandle_t uart_tx_handle = nullptr;

static uart_inst_t* uart_instance = nullptr;

/* stdio out_chars 回调：可从任意任务（及调度器启动前的 main）调用。
 * 队列满时仅让出 CPU；初始化阶段的打印远小于队列容量，不会触发。 */
static void uart_dma_write(const char *buf, int len) {
    std::string str(buf, len);
    for (;;) {
        bool pushed = false;
        MUTEX_LOCK(dma_tx_mutex) {
            if (dma_tx_queue.size() < IO_TX_QUEUE_SIZE) {
                dma_tx_queue.push_back(std::move(str));
                pushed = true;
            }
        }
        if (pushed) {
            break;
        }
        // 队列满：调度器运行中才让出（vTaskDelay 启动前不可用）
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            vTaskDelay(1);
        } else {
            tight_loop_contents();
        }
    }
    // 通知 TX 任务有数据待发送
    if (uart_tx_handle != nullptr) {
        xTaskNotifyGive(uart_tx_handle);
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

/* UART TX 队列泵：FreeRTOS 任务，阻塞等待数据后逐字节发送。 */
static void uart_tx_task(void* pv) {
    (void)pv;
    DEBUG_LOG("UART", "uart_tx task started");
    for (;;) {
        // 阻塞等待有数据（生产者 push 后 xTaskNotifyGive）
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 排空队列
        for (;;) {
            std::string str;
            bool got = false;
            MUTEX_LOCK(dma_tx_mutex) {
                if (!dma_tx_queue.empty()) {
                    str = std::move(dma_tx_queue.front());
                    dma_tx_queue.pop_front();
                    got = true;
                }
            }
            if (!got) {
                break;
            }
            for (char ch : str) {
                uart_putc_raw(uart_instance, ch);
            }
        }
    }
}

void UartDMAStdio::init(int pin_tx, int pin_rx, int baud_rate, uart_inst_t *uart, int dma) {
    uart_instance = uart ? uart : uart0;
    
    uart_init(uart_instance, baud_rate);
    gpio_set_function(pin_tx, GPIO_FUNC_UART);
    gpio_set_function(pin_rx, GPIO_FUNC_UART);
    
    // 创建 TX 泵任务（调度器启动前创建即可，启动后自动运行）
    xTaskCreate(uart_tx_task, "uart_tx", 512, nullptr, 2, &uart_tx_handle);
    
    stdio_set_driver_enabled(&uart_dma_stdio, true);

    DEBUG_LOG("UART", "DMA stdio init: tx=GP%d rx=GP%d baud=%d", pin_tx, pin_rx, baud_rate);
}

void UartDMAStdio::deinit() {
    stdio_set_driver_enabled(&uart_dma_stdio, false);
    uart_deinit(uart_instance);
}

