/* main.cpp
 * 入口：sys.init() -> sys.run()。
 * sys.init() 做一次性硬件初始化；sys.run() 创建 FreeRTOS 任务并启动
 * 双核 SMP 调度器（永不返回）。 */

#include "system.h"

int main()
{
    sys.init();   // 一次性初始化：UART stdio / USB HID / 矩阵 / LED
    sys.run();    // 创建任务 + 启动 FreeRTOS SMP 调度器（永不返回）
    return 0;
}
