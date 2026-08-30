cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(PICO_PLATFORM "rp2350-arm-s")
set(PICO_BOARD "super_rp2350b")
# Disable copy-to-RAM: keep .text resident in flash (XIP). Otherwise .text
# (~330KB) plus the FreeRTOS heap squeeze the C runtime heap down to 2KB, and
# std::vector/string/deque crash on any allocation.
# The RP2350 has an XIP cache; both cores fetch instructions directly, so there
# is no performance impact for a keyboard.
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

# ---- FreeRTOS SMP (Raspberry Pi fork's RP2350_ARM_NTZ port) ----
# FREERTOS_KERNEL_PATH: kernel root; FREERTOS_CONFIG_FILE_DIRECTORY: where FreeRTOSConfig.h lives
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/freertos/FreeRTOS-Kernel)
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/freertos)
add_subdirectory(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ FreeRTOS-Kernel)

# CONFIGURE_DEPENDS: re-scan the glob automatically when sources are added/removed
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directory
include_directories(include)

#pico_generate_pio_header(pico_all ${CMAKE_CURRENT_LIST_DIR}/ws2812b.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# USB HID keyboard: TinyUSB device stack (tusb_config.h in include/, HID driver enabled)
# pico_malloc: protect malloc/new with a mutex on multi-core (otherwise cross-core
# std::vector/string allocations crash)
target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib hardware_pio hardware_uart hardware_dma hardware_irq pico_multicore tinyusb_device pico_malloc FreeRTOS-Kernel-Heap4)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(pico_all)

set(CMAKE_SUPPRESS_REGENERATION false)
