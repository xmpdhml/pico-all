#pragma once

#include <stdio.h>

/* 调试日志总开关：1=开启，0=关闭。
 * 关闭时所有 DEBUG_LOG 展开为空（零开销，参数不参与求值）。
 * 注意：宿主单测构建通过 CMake 定义 DEBUG_LOG_ENABLE=0，避免刷屏。 */
#ifndef DEBUG_LOG_ENABLE
#define DEBUG_LOG_ENABLE 1
#endif

/* DEBUG_LOG(tag, fmt, ...)：输出 "[tag] msg\n" 到 stdio。
 * stdio 经 UART DMA 队列（uart_dma_stdio）转发，跨任务调用安全。
 * 例：DEBUG_LOG("SYS", "init done, rows=%d", n); */
#if DEBUG_LOG_ENABLE
    #define DEBUG_LOG(tag, fmt, ...) \
        do { printf("[%s] " fmt "\n", tag, ##__VA_ARGS__); } while (0)
#else
    #define DEBUG_LOG(tag, fmt, ...) ((void)0)
#endif
