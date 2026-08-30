#pragma once

#include <vector>

#include "key_matrix.h"
#include "matrix_io.h"

/* KeyScanner: matrix scan + debounce engine.
 *
 * - init(): initialize the matrix (forwarded to MatrixIO);
 * - scan(): read every cell + per-key counting debounce, producing three key
 *   lists (basic/extended/internal) and their change flags; called once per
 *   main-loop iteration.
 *
 * Hardware access (row driving / column reading / pull-ups) is injected via
 * MatrixIO, so this class can be validated on the host with a simulated matrix
 * (SimMatrixIO) for debounce and classification logic.
 *
 * Debounce: a state is only stable after kDebounceThreshold consecutive scans
 * read the same value; the threshold corresponds to ~4× the scan period
 * (~40ms at a 10ms main loop).
 * Keymap and pins come from KeyMatrix (this class only handles "how to scan",
 * not "which key is produced").
 */

class KeyScanner
{
    friend class System;   // Only System can create it (same pattern as UartDMAStdio)

public:
    void init();   // Initialize the matrix (forwarded to MatrixIO)
    void scan();   // Scan + debounce (called once per main-loop iteration)

    // Current frame results
    const std::vector<KeyCodes>& pressed_basic() const    { return pressed_basic_; }
    const std::vector<KeyCodes>& pressed_extended() const { return pressed_extended_; }
    const std::vector<KeyCodes>& pressed_internal() const { return pressed_internal_; }
    bool basic_changed() const    { return basic_changed_; }
    bool extended_changed() const { return extended_changed_; }
    bool internal_changed() const { return internal_changed_; }

private:
    KeyScanner(const KeyMatrix& matrix, MatrixIO& io);
    ~KeyScanner() = default;
    KeyScanner(const KeyScanner&) = delete;
    KeyScanner& operator=(const KeyScanner&) = delete;
    KeyScanner(KeyScanner&&) = delete;
    KeyScanner& operator=(KeyScanner&&) = delete;

    static constexpr uint8_t kDebounceThreshold = 4;

    void add_pressed(KeyCodes code);

    const KeyMatrix& matrix_;
    MatrixIO&        io_;
    std::vector<uint8_t> debounce_;          // [rows*cols] debounce counters
    std::vector<KeyCodes> last_basic_;       // Previous frame (for change flags)
    std::vector<KeyCodes> last_extended_;
    std::vector<KeyCodes> last_internal_;

    std::vector<KeyCodes> pressed_basic_;
    std::vector<KeyCodes> pressed_extended_;
    std::vector<KeyCodes> pressed_internal_;
    bool basic_changed_ = false;
    bool extended_changed_ = false;
    bool internal_changed_ = false;
};
