#include "key_matrix.h"

KeyMatrix::KeyMatrix(const Config& cfg)
    : rows_(cfg.rows),
      cols_(cfg.cols),
      row_pins_(cfg.row_pins),
      col_pins_(cfg.col_pins),
      keymap_(cfg.keymap),
      direction_(cfg.direction),
      pullup_(cfg.pullup)
{
}
