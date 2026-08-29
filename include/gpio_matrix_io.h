#pragma once

#include "matrix_io.h"
#include "key_matrix.h"

/* GpioMatrixIO：真实 GPIO 实现（只支持 pullup 方案）。
 * 行输出、列输入上拉；激活行拉低，按下时列读到 0。 */

class GpioMatrixIO : public MatrixIO
{
    friend class System;   // 仅 System 可创建（参照 UartDMAStdio 模式）

public:
    void init() override;
    bool read_cell(uint8_t row, uint8_t col) const override;

private:
    explicit GpioMatrixIO(const KeyMatrix& matrix);
    ~GpioMatrixIO() = default;
    GpioMatrixIO(const GpioMatrixIO&) = delete;
    GpioMatrixIO& operator=(const GpioMatrixIO&) = delete;
    GpioMatrixIO(GpioMatrixIO&&) = delete;
    GpioMatrixIO& operator=(GpioMatrixIO&&) = delete;

    const KeyMatrix& matrix_;
};
