cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM "rp2350-arm-s")
set(PICO_BOARD "super_rp2350b")
set(PICO_COPY_TO_RAM 1)
set(PROJECT_NAME pico_all)
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(${PROJECT_NAME} LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directory
include_directories(include)

#pico_generate_pio_header(pico_all ${CMAKE_CURRENT_LIST_DIR}/ws2812b.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib hardware_pio hardware_uart hardware_dma hardware_irq)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(pico_all)

set(CMAKE_SUPPRESS_REGENERATION false)
