# pico-all —— RP2350B USB HID 键盘固件

基于 **Raspberry Pi RP2350B**（双核 Cortex-M33）的 USB HID 键盘固件，运行 **FreeRTOS SMP 双核** + **TinyUSB** 设备栈，用 **C++23** 编写。

- 双核 SMP 任务调度（扫描 / HID / 系统 / UART 各自独立任务）
- USB HID 键盘：6KRO / NKRO 可切换 + Consumer 媒体/系统/应用启动报告
- 按键扫描 + 计数去抖，逻辑与硬件解耦（可宿主单测）
- UART DMA 队列式调试日志（多任务安全）
- 目标矩阵 6×18（108 键），当前为 3×3 占位表

## 硬件

- **目标板**：Nologo Super RP2350B（RP2350B，48 GPIO）
  - 16MB QSPI Flash（XIP）、8MB PSRAM（CS 接 GP47，暂未启用）
  - 无板载 LED / 无按键 / 无 USB 座，需自接
  - 进入 UF2 下载模式：**上电前短接 `BS` 焊盘与 `GND`**
- **当前接线**（占位，集中定义在 `include/keyboard_config.h`）：

  | 功能 | GPIO |
  |---|---|
  | 调试 UART TX / RX | GP0 / GP1（1.5M baud） |
  | 状态 LED | GP2 |
  | 矩阵行（输出） | GP32 / GP34 / GP36 |
  | 矩阵列（输入，上拉） | GP38 / GP40 / GP42 |
  | 旋转编码器（预留） | GP29 / GP30 / GP31 |

> 目标 6×18 布局（行 GP0–5，列 GP6–11 + GP14–25）模板见 `keyboard/keyboard_config.h`（参考项目）。

## 目录结构

```
pico-all/
├── CMakeLists.txt           # 构建配置（含 FreeRTOS 接入）
├── boards/
│   └── super_rp2350b.h      # 板级定义（16MB Flash / PSRAM 等）
├── include/
│   ├── keyboard_config.h    # ★ 统一配置：引脚 + 键位（改接线/键位只动这里）
│   ├── key_codes.h          # 键码枚举（KeyCodes）
│   ├── key_matrix.h         # 矩阵静态描述
│   ├── matrix_io.h          # 矩阵 IO 抽象（可测性）
│   ├── gpio_matrix_io.h     # 矩阵 IO 的真机实现
│   ├── key_scan.h           # 扫描 + 去抖
│   ├── usb_hid.h            # HID 报告编码 / 上报 / 模式切换
│   ├── uart_dma_stdio.h     # UART 队列式 stdio（调试输出）
│   ├── system.h             # System 单例（组合所有子系统 + RTOS 任务）
│   ├── debug_log.h          # DEBUG_LOG 调试日志宏
│   ├── mutex_wrap.h         # 互斥锁封装
│   ├── tusb_config.h        # TinyUSB 配置
│   └── usb_descriptors.h    # USB 描述符
├── src/                     # 实现（与 include 一一对应 + main.cpp）
├── freertos/
│   ├── FreeRTOSConfig.h     # FreeRTOS 配置（SMP 双核）
│   └── FreeRTOS-Kernel/     # git submodule（树莓派 fork，RP2350_ARM_NTZ 端口）
└── test/                    # 宿主机单元测试（独立 CMake 工程）
```

## 依赖

- **pico-sdk** 2.3.0+（构建时通过 `PICO_SDK_PATH` 指定）
- **FreeRTOS-Kernel**（submodule，树莓派 fork，`RP2350_ARM_NTZ` 端口）
- 交叉工具链：`arm-none-eabi-gcc` / `arm-none-eabi-g++`

## 构建

```bash
# 1. 指定 SDK 路径
export PICO_SDK_PATH=/path/to/pico-sdk

# 2. 拉取 FreeRTOS 子模块
cd pico-all
git submodule update --init

# 3. 配置 + 编译
cmake -B build
make -C build -j8
```

产物：`build/pico_all.uf2`

## 烧录

1. 断电，短接 `BS` 焊盘与 `GND`；
2. 接 USB 上电，电脑出现 UF2 磁盘；
3. 把 `build/pico_all.uf2` 拖入该磁盘（或用 `picotool load -f build/pico_all.uf2`）。

## 配置

改硬件只需要动 `include/keyboard_config.h`：

| 内容 | 位置 |
|---|---|
| 引脚（UART / LED / 编码器 / 矩阵） | `keyboard_config.h` → `keyboard_config::k*Pin` |
| 键位映射表 | `keyboard_config.h` → `keyboard_config::kKeymap` |
| 矩阵行列数 | `keyboard_config.h` → `keyboard_config::kRows/kCols` |
| FreeRTOS 调度参数 | `freertos/FreeRTOSConfig.h` |
| USB 描述符 / HID | `include/tusb_config.h`、`include/usb_descriptors.h` |

## 按键功能（内部功能键）

在 `kKeymap` 中放入以下键码即可触发（键码定义见 `key_codes.h`）：

| 键码 | 功能 |
|---|---|
| `KEY_P_NKRO_ON_OFF` | 切换 6KRO / NKRO |
| `KEY_P_NKRO` / `KEY_P_6KRO` | 强制 NKRO / 6KRO |
| `KEY_P_USB_BURN` | 进入 UF2 引导（软件进 bootloader） |
| `KEY_P_REBOOT` | 软复位 |

## FreeRTOS 任务模型

| 任务 | 优先级 | 周期 | 职责 |
|---|---|---|---|
| `keyboard` | 4 | 10ms | 扫描 + HID 上报 + 内部动作 |
| `system` | 1 | 10ms | LED 心跳 + 控制台 |
| `uart_tx` | 2 | 事件驱动 | UART TX 队列泵 |

> 扫描与 HID 在同一任务内完成，避免跨核共享扫描器状态；UART 输出经队列 + 互斥锁，任意任务可安全 `printf`/`DEBUG_LOG`。

## 单元测试（宿主机）

`test/` 是零依赖的宿主测试工程（不依赖 pico-sdk，用 `SimMatrixIO` 模拟矩阵、`tusb_stub` 替身）：

```bash
cd test
cmake -B build
cmake --build build
./build/pico_all_tests
```

覆盖：矩阵几何 / 扫描去抖与分类 / HID 键码映射与报告构建 / NKRO 切换。

## 调试日志

用 `DEBUG_LOG("TAG", "fmt", ...)` 输出（经 UART DMA 队列，跨任务安全）：

```cpp
#include "debug_log.h"
DEBUG_LOG("SYS", "init done, rows=%d", n);   // → [SYS] init done, rows=3
```

- 总开关 `DEBUG_LOG_ENABLE`（`debug_log.h`，默认 1，发布可置 0）；
- 注意：日志必须放在 `UartDMAStdio` 初始化之后才会输出（stdio 驱动启用前输出被丢弃）。
