#pragma once

#include <cstdint>

/* MatrixIO: abstraction of the matrix "logical read".
 *
 * Lets KeyScanner avoid direct GPIO dependency, so a simulated matrix
 * (SimMatrixIO, see test/) can be injected for host unit tests. The real
 * GpioMatrixIO handles row driving / column reading / pull-ups etc. */

class MatrixIO
{
public:
    virtual ~MatrixIO() = default;

    virtual void init() = 0;                                    // Initialize matrix pins
    virtual bool read_cell(uint8_t row, uint8_t col) const = 0; // Whether this cell conducts
};
