#pragma once

#include "matrix_io.h"
#include "key_matrix.h"

/* GpioMatrixIO: real GPIO implementation (pull-up scheme only).
 * Rows are outputs, columns are inputs with pull-up; the active row is driven
 * low, and a pressed key reads 0 on the column. */

class GpioMatrixIO : public MatrixIO
{
    friend class System;   // Only System can create it (same pattern as UartDMAStdio)

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
