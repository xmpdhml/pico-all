cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(PICO_PLATFORM "rp2350-arm-s")
set(PICO_BOARD "super_rp2350b")
# 关闭 copy-to-RAM：让 .text 常驻 flash(XIP)。否则 .text(约330KB)+FreeRTOS 堆
# 会把 C 运行时堆挤到只剩 2KB，std::vector/string/deque 一分配就崩。
# RP2350 有 XIP 缓存，双核均可直接取指执行，键盘场景无性能影响。
set(PICO_COPY_TO_RAM 0)
set(PROJECT_NAME pico_all)
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(${PROJECT_NAME} LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

pico_sdk_init()

# ---- FreeRTOS SMP（树莓派 fork 的 RP2350_ARM_NTZ 端口） ----
# FREERTOS_KERNEL_PATH：内核根目录；FREERTOS_CONFIG_FILE_DIRECTORY：FreeRTOSConfig.h 所在目录
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/freertos/FreeRTOS-Kernel)
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/freertos)
add_subdirectory(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ FreeRTOS-Kernel)

# CONFIGURE_DEPENDS：新增/删除源文件时自动重新扫描 glob，无需手动重新 cmake -B
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directory
include_directories(include)

#pico_generate_pio_header(pico_all ${CMAKE_CURRENT_LIST_DIR}/ws2812b.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# USB HID 键盘：TinyUSB 设备栈（tusb_config.h 在 include/ 中，已启用 HID 驱动）
# pico_malloc：多核下用互斥锁保护 malloc/new（否则 std::vector/string 跨核分配会崩）
target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib hardware_pio hardware_uart hardware_dma hardware_irq pico_multicore tinyusb_device pico_malloc FreeRTOS-Kernel-Heap4)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(pico_all)

set(CMAKE_SUPPRESS_REGENERATION false)
