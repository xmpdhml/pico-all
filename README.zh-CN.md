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
3. 把 `build/pico_all.uf2` 拖入该磁盘（或用 `picotool load -f build/pico_all.uf2`）；
4. **断开 `BS` 与 `GND` 的短接，再重新上电** —— 若一直短接着，下次上电仍会进入 UF2 引导模式（表现为一个名为 `RP2350` 的 U 盘，而不是键盘）。

## 上电后应该看到什么（自检）

USB 上应该出现**一个 HID 键盘**：

| 项目 | 预期值 |
|---|---|
| 设备类型 | USB HID 键盘（Windows 设备管理器 → “键盘” → `HID Keyboard Device`） |
| 硬件 ID | `USB\VID_CAFE&PID_0001` |
| 产品 / 厂商字符串 | `RP2350B HID Keyboard` / `MyKeyboard` |
| 序列号 | 芯片唯一 ID（`pico_get_unique_board_id_string()` 生成） |
| HID 报告 | 一个接口 5 个报告：6KRO、NKRO 位图、媒体键、系统键、AL_* 启动键 |
| **不会**出现 | COM 口（stdio USB 已关闭）、U 盘 |

**不按任何键本来“没反应”**：当前 `kKeymap` 是 3×3 占位表，行列接在 GP32/34/36 与 GP38/40/42，没接开关就永远不会产生按键。

不接开关也能验证整条链路：

| 检查 | 方法 | 预期 |
|---|---|---|
| 固件是否在跑 | GP2 接 LED | 约 50Hz 闪烧（半边亮） |
| 矩阵 → HID 通路 | 短接 **GP32 与 GP38** | 打出一个小写 `a` |
| USB 枚举是否成功 | 看 UART 日志 | 出现 `USB mounted by host: enumeration complete` || 主机 → 设备通路 | 在**另一个键盘**上按 Caps Lock | 日志出现 `LED state from host: 0x02 (num=0 caps=1 ...)` |
UART（GP0=TX, GP1=RX, **1500000** 8N1）正常启动依次输出：

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
[HID] USB mounted by host: enumeration complete   ← 只有这行代表枚举成功
```

> `[BOOT]` 开头的行走的是**不依赖 RTOS 的直接阻塞写**（`uart_boot_puts()`），
> 所以在调度器启动失败时也能看到 —— 它们是判断“卡在哪一步”的关键。

### UART 完全没有输出

说明固件在 `vTaskStartScheduler()` 之前就卡住了（正常 printf 靠 `uart_tx` 任务泵出，调度器不启动就一个字都打不出来）：

- 停在 `[BOOT] firmware running` 之后：卡在 `System::init()` 后半段。
- 出现 `*** FATAL: vTaskStartScheduler() returned`：FreeRTOS 堆不足（`configTOTAL_HEAP_SIZE`）。
- 出现 `*** FreeRTOS configASSERT FAILED at <文件>:<行> ***`：按提示定位。

> ⚠️ **调度器启动前绝不能调用 `sleep_ms()` / `sleep_us()`**：开了
> `configSUPPORT_PICO_TIME_INTEROP=1` 后它们会落到 `vTaskDelay()`，而调度器未启动时
> 没有 current TCB，直接是非法操作、启动就此中止。延时请用 `busy_wait_ms()`。
> 同理，启动前的 `printf` 也不能走 RTOS 队列（现已在 `uart_dma_write` 里自动退回直写）。

### 枚举成功但报「代码 10 / 报表未对齐字节」

HID 报表描述符没通过 Windows 校验（枚举本身已成功，所以能看到 `USB mounted by host`）。
逐项核对：**每个报表的位宽必须是 8 的倍数，且与固件实发字节数一致**，位图报表记得补常量位。
用脚本直接检查 ELF 里的真实描述符定位问题：

```bash
python3 test/check_hid_descriptor.py
```

### 插上完全没反应时

1. **是否还停在 UF2 引导模式** —— `BS`–`GND` 是否还短接着？断开后重新上电；此时电脑里出现的是 U 盘而非键盘。
2. **Windows 复用了旧的失败记录** —— 之前出过代码 43，设备管理器 → 勾选“显示隐藏的设备” → 卸载所有 `VID_CAFE&PID_0001` 条目（含灰色）→ 重新插拔。序列号现已改为芯片唯一 ID，正常情况下会建新的设备节点。
3. **USB 数据线接反 / 虚焊** —— 绿=U+=D+、白=U-=D-；确认 VB 有 5V、与主机共地。
4. **换口换线** —— 直连主机，不要走 hub。
5. 若 UART 日志停在 `tusb init done` 之前，说明问题在固件启动阶段（看日志最后一行卡在哪）。

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

### HID 描述符校验（防「代码 10 / 报表未对齐字节」）

Windows 会校验 HID 报表描述符：**任一报表的位宽不是 8 的倍数，就会直接拒绝设备**（枚举能过，但报代码 10）。位图型报表特别容易踩坑 —— 比如 3 bit 的系统键、67 bit 的 AL_* 都必须补上常量位凑成整字节，而且**描述符声明的宽度必须和固件实际发送的字节数一致**（这两处是各自独立的，不同步时只有上真机才会暴露）。

脚本会直接从 ELF 里取出**真实描述符字节**逐项校验（不依赖硬件，也不需要重新编译）：

```bash
python3 test/check_hid_descriptor.py        # 默认检查 build/pico_all.elf，可传入其它 elf 路径
```

输出示例：

```
 ID  IN bits  IN bytes       OUT bits  OUT bytes
  1       64         8   OK         8          1   OK
  2      272        34   OK         0          0    -
  ...
  input  id4: descriptor    8 bit vs firmware    8 bit -> OK

OK: all reports are byte-aligned and match the firmware
```

只要有报表没对齐、或声明宽度与实发宽度不符，就会列出问题并以非零退出码失败。

> 改动「报表描述符」或 `CONSUMER_*_BITMAP_SIZE` 等宏之后请重跑一次；
> 同时 `src/usb_descriptors.c` 里带了 `_Static_assert` 兜住位图放不下使用范围的情况。

## 调试日志

所有调试输出都由 `include/debug_log.h` 里的**编译期开关**统一控制。关掉后宏展开为空——不产生任何代码、参数不求值、零开销。

| 开关 | 默认 | 控制范围 |
|---|---|---|
| `DEBUG_LOG_ENABLE` | 1 | 总开关：`DEBUG_LOG()` 以及 `system.cpp` 里零散的 `printf`/`std::cout` 调试行 |
| `BOOT_LOG_ENABLE` | 跟随总开关 | `[BOOT]` 启动里程碑（`BOOT_LOG()`） |
| `FATAL_LOG_ENABLE` | 1 | 仅致命诊断（`configASSERT` 失败、"调度器启动失败"） |

三个输出宏：

```cpp
#include "debug_log.h"

DEBUG_LOG("SYS", "init done, rows=%d", n);    // → [SYS] init done, rows=3
                                              // 走 UART DMA 队列：任意任务可安全调用，
                                              // 但需要调度器已运行（由 uart_tx 任务泵出）
BOOT_LOG("[BOOT] keyboard task running\r\n"); // 直接阻塞写 UART：调度器启动前、
                                              // 以及启动失败时同样能看到
FATAL_LOG("...");                             // 只用在致命错误路径
```

不改代码即可编出「安静」的发布固件：

```bash
cmake -B build -DCMAKE_C_FLAGS="-DDEBUG_LOG_ENABLE=0" \
                -DCMAKE_CXX_FLAGS="-DDEBUG_LOG_ENABLE=0"
```

- `BOOT_LOG()` 自动跟随总开关，所以上面这一个宏就能同时静音普通日志和启动里程碑。
- `FATAL_LOG_ENABLE` **默认保持开启是刻意的**：它只在致命错误时才可能输出，平时零开销；而一旦关掉，启动失败就变成「完全没有任何输出」，极难排查（我们刚踩过这个坑）。要彻底静音再加 `-DFATAL_LOG_ENABLE=0`。
- 注意：日志必须放在 `UartDMAStdio` 初始化之后才会输出（stdio 驱动启用前输出被丢弃）。
