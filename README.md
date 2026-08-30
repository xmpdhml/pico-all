# pico-all — RP2350B USB HID Keyboard Firmware

A USB HID keyboard firmware for the **Raspberry Pi RP2350B** (dual-core Cortex-M33), running **FreeRTOS SMP** + the **TinyUSB** device stack, written in **C++23**.

- Dual-core SMP task scheduling (scan / HID / system / UART each in its own task)
- USB HID keyboard: switchable 6KRO / NKRO + Consumer media / system / application-launch reports
- Key scanning with counting debounce, decoupled from hardware (host unit-testable)
- UART queue-based debug logging (safe across tasks)
- Target matrix 6×18 (108 keys); currently a 3×3 placeholder

## Hardware

- **Target board**: Nologo Super RP2350B (RP2350B, 48 GPIO)
  - 16MB QSPI Flash (XIP), 8MB PSRAM (CS on GP47, not yet enabled)
  - No on-board LED / button / USB connector — bring your own
  - UF2 download mode: **short `BS` pad to `GND` before power-on**
- **Current wiring** (placeholder, centralized in `include/keyboard_config.h`):

  | Function | GPIO |
  |---|---|
  | Debug UART TX / RX | GP0 / GP1 (1.5M baud) |
  | Status LED | GP2 |
  | Matrix rows (output) | GP32 / GP34 / GP36 |
  | Matrix columns (input, pull-up) | GP38 / GP40 / GP42 |
  | Rotary encoder (reserved) | GP29 / GP30 / GP31 |

> The target 6×18 layout (rows GP0–5, columns GP6–11 + GP14–25) template lives in `keyboard/keyboard_config.h` (reference project).

## Directory Layout

```
pico-all/
├── CMakeLists.txt           # Build config (incl. FreeRTOS integration)
├── boards/
│   └── super_rp2350b.h      # Board definitions (16MB Flash / PSRAM etc.)
├── include/
│   ├── keyboard_config.h    # ★ Unified config: pins + keymap (edit only this for wiring/keymap)
│   ├── key_codes.h          # Keycode enum (KeyCodes)
│   ├── key_matrix.h         # Static matrix description
│   ├── matrix_io.h          # Matrix IO abstraction (testability)
│   ├── gpio_matrix_io.h     # Real GPIO implementation of matrix IO
│   ├── key_scan.h           # Scan + debounce
│   ├── usb_hid.h            # HID report encoding / sending / mode switch
│   ├── uart_dma_stdio.h     # UART queue-based stdio (debug output)
│   ├── system.h             # System singleton (composes subsystems + RTOS tasks)
│   ├── debug_log.h          # DEBUG_LOG macro
│   ├── mutex_wrap.h         # Mutex wrapper
│   ├── tusb_config.h        # TinyUSB config
│   └── usb_descriptors.h    # USB descriptors
├── src/                     # Implementations (mirror include/ + main.cpp)
├── freertos/
│   ├── FreeRTOSConfig.h     # FreeRTOS config (SMP dual-core)
│   └── FreeRTOS-Kernel/     # git submodule (Raspberry Pi fork, RP2350_ARM_NTZ port)
└── test/                    # Host unit tests (standalone CMake project)
```

## Dependencies

- **pico-sdk** 2.3.0+ (provided via `PICO_SDK_PATH`)
- **FreeRTOS-Kernel** (submodule, Raspberry Pi fork, `RP2350_ARM_NTZ` port)
- Cross toolchain: `arm-none-eabi-gcc` / `arm-none-eabi-g++`

## Building

```bash
# 1. Point to the SDK
export PICO_SDK_PATH=/path/to/pico-sdk

# 2. Fetch the FreeRTOS submodule
cd pico-all
git submodule update --init

# 3. Configure + build
cmake -B build
make -C build -j8
```

Artifact: `build/pico_all.uf2`

## Flashing

1. Power off, short the `BS` pad to `GND`;
2. Connect USB and power on — a UF2 drive appears;
3. Drag `build/pico_all.uf2` onto the drive (or use `picotool load -f build/pico_all.uf2`).

## Configuration

Hardware changes only require editing `include/keyboard_config.h`:

| What | Where |
|---|---|
| Pins (UART / LED / encoder / matrix) | `keyboard_config.h` → `keyboard_config::k*Pin` |
| Keymap | `keyboard_config.h` → `keyboard_config::kKeymap` |
| Matrix dimensions | `keyboard_config.h` → `keyboard_config::kRows/kCols` |
| FreeRTOS scheduling params | `freertos/FreeRTOSConfig.h` |
| USB descriptors / HID | `include/tusb_config.h`, `include/usb_descriptors.h` |

## Internal Function Keys

Place these keycodes in `kKeymap` to trigger them (definitions in `key_codes.h`):

| Keycode | Action |
|---|---|
| `KEY_P_NKRO_ON_OFF` | Toggle 6KRO / NKRO |
| `KEY_P_NKRO` / `KEY_P_6KRO` | Force NKRO / 6KRO |
| `KEY_P_USB_BURN` | Enter UF2 bootloader (software) |
| `KEY_P_REBOOT` | Soft reset |

## FreeRTOS Task Model

| Task | Priority | Period | Responsibility |
|---|---|---|---|
| `keyboard` | 4 | 10ms | Scan + HID report + internal actions |
| `system` | 1 | 10ms | LED heartbeat + console |
| `uart_tx` | 2 | Event-driven | UART TX queue pump |

> Scanning and HID run in the same task, avoiding cross-core sharing of scanner state; UART output goes through a queue + mutex, so any task can safely `printf`/`DEBUG_LOG`.

## Unit Tests (Host)

`test/` is a zero-dependency host test project (no pico-sdk; uses `SimMatrixIO` to simulate the matrix and a `tusb_stub` stand-in):

```bash
cd test
cmake -B build
cmake --build build
./build/pico_all_tests
```

Covers: matrix geometry / scan debounce & classification / HID keycode mapping & report building / NKRO switching.

## Debug Logging

Use `DEBUG_LOG("TAG", "fmt", ...)` (routed through the UART queue, safe across tasks):

```cpp
#include "debug_log.h"
DEBUG_LOG("SYS", "init done, rows=%d", n);   // → [SYS] init done, rows=3
```

- Master switch `DEBUG_LOG_ENABLE` (`debug_log.h`, default 1; set 0 for release);
- Note: log only after `UartDMAStdio` is initialized (output is discarded before the stdio driver is enabled).

