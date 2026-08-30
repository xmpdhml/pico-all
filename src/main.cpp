/* main.cpp
 * 入口：sys.init() -> sys.run()。
 * sys.init() 做一次性硬件初始化；sys.run() 创建 FreeRTOS 任务并启动
 * 双核 SMP 调度器（永不返回）。 */

#include "system.h"
#include "debug_log.h"

int main()
{
    // 注意：sys.init() 之前 UART stdio 尚未启用，此时 DEBUG_LOG 输出会被静默丢弃，
    // 故首个日志放在 init() 之后。
    sys.init();   // 一次性初始化：UART stdio / USB HID / 矩阵 / LED
    DEBUG_LOG("MAIN", "boot");
    DEBUG_LOG("MAIN", "init done, entering scheduler");
    sys.run();    // 创建任务 + 启动 FreeRTOS SMP 调度器（永不返回）
    return 0;
}
