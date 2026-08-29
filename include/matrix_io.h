#pragma once

#include <cstdint>

/* MatrixIO：矩阵"逻辑读"抽象。
 *
 * 让 KeyScanner 不直接依赖 GPIO，从而可以在宿主机单元测试中注入
 * 模拟矩阵（SimMatrixIO，见 test/）。真机实现 GpioMatrixIO 负责
 * 行驱动 / 列读取 / 上拉等硬件细节。
 */

class MatrixIO
{
public:
    virtual ~MatrixIO() = default;

    virtual void init() = 0;                                    // 初始化矩阵引脚
    virtual bool read_cell(uint8_t row, uint8_t col) const = 0; // 该格是否导通
};
