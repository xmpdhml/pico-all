#include "gpio_matrix_io.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"   // sleep_us

#include "debug_log.h"

GpioMatrixIO::GpioMatrixIO(const KeyMatrix& matrix)
    : matrix_(matrix)
{
}

void GpioMatrixIO::init()
{
    DEBUG_LOG("MATRIX", "gpio matrix io init: %dx%d (rows out, cols pull-up)",
              matrix_.rows(), matrix_.cols());

    // Output side (rows): drive high (inactive) by default
    for (uint8_t r = 0; r < matrix_.rows(); r++) {
        gpio_init(matrix_.row_pin(r));
        gpio_set_dir(matrix_.row_pin(r), GPIO_OUT);
        gpio_put(matrix_.row_pin(r), 1);
    }

    // Input side (columns): pull up uniformly (pull-up scheme only)
    for (uint8_t c = 0; c < matrix_.cols(); c++) {
        gpio_init(matrix_.col_pin(c));
        gpio_set_dir(matrix_.col_pin(c), GPIO_IN);
        gpio_pull_up(matrix_.col_pin(c));
    }
}

bool GpioMatrixIO::read_cell(uint8_t row, uint8_t col) const
{
    // Drive the active row low, read the column (pressed reads 0), release the row
    gpio_put(matrix_.row_pin(row), 0);
    sleep_us(5);
    const bool pressed = (gpio_get(matrix_.col_pin(col)) == 0);
    gpio_put(matrix_.row_pin(row), 1);
    return pressed;
}
