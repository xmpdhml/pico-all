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
3. Drag `build/pico_all.uf2` onto the drive (or use `picotool load -f build/pico_all.uf2`);
4. **Remove the `BS`–`GND` short and power-cycle.** If `BS` stays shorted, the next power-on
   enters the UF2 bootloader again — the board then shows up as a removable drive named
   `RP2350`, not as a keyboard.

## What you should see once powered (self-check)

A **single USB HID keyboard** should enumerate:

| Item | Expected |
|---|---|
| Device type | USB HID keyboard (Windows Device Manager → Keyboards → `HID Keyboard Device`) |
| Hardware ID | `USB\VID_CAFE&PID_0001` |
| Product / manufacturer strings | `RP2350B HID Keyboard` / `MyKeyboard` |
| Serial number | Chip unique ID (from `pico_get_unique_board_id_string()`) |
| HID reports | One interface with 5 reports: 6KRO, NKRO bitmap, media, system, AL_* launch |
| **Not** present | COM port (USB stdio is disabled), removable drive |

**Nothing happening when you type is expected**: `kKeymap` is currently a 3×3 placeholder wired
to GP32/34/36 (rows) and GP38/40/42 (cols) — with no switches attached it can never emit a key.

You can verify the whole chain without any switches:

| Check | How | Expected |
|---|---|---|
| Firmware is alive | LED on GP2 | ~50 Hz blink (half brightness) |
| Matrix → HID path | Short **GP32 to GP38** | Types a lowercase `a` |
| USB enumeration | Watch the UART log | `USB mounted by host: enumeration complete` |

UART (GP0=TX, GP1=RX, **1500000** 8N1) should print, in order:

```
[BOOT] firmware running; UART ready
Hello, world! customized
[SYS] init begin
[SYS] matrix scan init done (3x3)
[SYS] LED init done (GP2)
[BOOT] init done; about to start scheduler
Starting FreeRTOS SMP scheduler...
[SYS] creating tasks (keyboard prio4, system prio1)
[SYS] starting FreeRTOS SMP scheduler on 2 cores
[BOOT] calling vTaskStartScheduler()
[BOOT] keyboard task running
[HID] tusb init done (core 0)
[BOOT] USB initialized
[SCAN] keyboard task running on core 0
[HID] USB mounted by host: enumeration complete   ← only this line proves enumeration
```

> Lines starting with `[BOOT]` use a direct blocking write that does **not** depend on
> FreeRTOS (`uart_boot_puts()`), so they still appear when scheduler startup fails —
> they are what tells you which step it stalled at.

### No UART output at all

The firmware stalled before `vTaskStartScheduler()` (normal printf output is pumped by the
`uart_tx` task, so it produces nothing while the scheduler is down):

- Stops right after `[BOOT] firmware running`: it hung in the second half of `System::init()`.
- `*** FATAL: vTaskStartScheduler() returned`: not enough FreeRTOS heap (`configTOTAL_HEAP_SIZE`).
- `*** FreeRTOS configASSERT FAILED at <file>:<line> ***`: look at that location.

> ⚠️ **Never call `sleep_ms()` / `sleep_us()` before the scheduler starts.** With
> `configSUPPORT_PICO_TIME_INTEROP=1` they resolve to `vTaskDelay()`, which is invalid
> without a current TCB and aborts startup. Use `busy_wait_ms()` instead. Likewise, pre-startup
> `printf` must not go through the RTOS queue (handled automatically in `uart_dma_write`).

### Enumeration succeeds but Windows reports Code 10 ("report is not byte-aligned")

The HID report descriptor failed Windows' validation (enumeration itself worked, which is why
`USB mounted by host` appears). Check that **every report's bit width is a multiple of 8 and
matches the byte count the firmware sends**, padding bit-field reports where needed. Use the
script to inspect the real descriptor in the ELF:

```bash
python3 test/check_hid_descriptor.py
```

### When plugging in produces no reaction at all

1. **Still in UF2 bootloader?** Is `BS` still shorted to `GND`? Remove the short and power-cycle;
   in that state the host sees a drive, not a keyboard.
2. **Windows reused a cached failure.** After the earlier Code 43, open Device Manager → enable
   *Show hidden devices* → uninstall every `VID_CAFE&PID_0001` entry (including greyed-out ones) →
   replug. The serial is now the chip unique ID, so a fresh device node should be created.
3. **USB data lines swapped / cold joint** — green = U+ = D+, white = U- = D-; verify VB has 5 V
   and that ground is shared with the host.
4. **Try another port/cable** — connect directly, not through a hub.
5. If the UART log stops before `tusb init done`, the problem is in firmware startup (the last log
   line shows where it stalled).

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

### HID descriptor validation (guards against "Code 10 / report is not byte-aligned")

Windows validates the HID report descriptor: **if any report's bit width is not a multiple of
8 it rejects the device outright** — enumeration succeeds, then the device fails with Code 10.
Bit-field reports are the usual trap: a 3-bit system-keys report or a 67-bit AL_* report must be
padded with constant bits to a whole byte, and the width declared in the descriptor must match
the number of bytes the firmware actually sends (two independent places, so a mismatch only
shows up on real hardware).

The script extracts the **real descriptor bytes** from the ELF and checks both rules — no
hardware and no rebuild needed:

```bash
python3 test/check_hid_descriptor.py        # defaults to build/pico_all.elf
```

```
 ID  IN bits  IN bytes       OUT bits  OUT bytes
  1       64         8   OK         8          1   OK
  2      272        34   OK         0          0    -
  ...
  input  id4: descriptor    8 bit vs firmware    8 bit -> OK

OK: all reports are byte-aligned and match the firmware
```

It lists the problem and exits non-zero if a report is misaligned or a declared width does not
match. Re-run it after touching the report descriptor or the `CONSUMER_*_BITMAP_SIZE` macros;
`src/usb_descriptors.c` also carries `_Static_assert`s guarding the bitmap sizes.

## Debug Logging

Use `DEBUG_LOG("TAG", "fmt", ...)` (routed through the UART queue, safe across tasks):

```cpp
#include "debug_log.h"
DEBUG_LOG("SYS", "init done, rows=%d", n);   // → [SYS] init done, rows=3
```

- Master switch `DEBUG_LOG_ENABLE` (`debug_log.h`, default 1; set 0 for release);
- Note: log only after `UartDMAStdio` is initialized (output is discarded before the stdio driver is enabled).

