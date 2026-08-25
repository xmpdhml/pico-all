/* main.cpp
 * 入口：sys.init() -> sys.run()。
 * 所有初始化与主循环逻辑都收进 System（system.h/system.cpp），
 * 这里只做启动两件事。 */

#include "system.h"

int main()
{
    sys.init();   // 一次性初始化：UART stdio / USB HID / 矩阵 / LED
    sys.run();    // 主循环（永不返回）
    return 0;
}
