cmake_minimum_required(VERSION 3.13)
project(pico_all_tests CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 宿主机单测：编译与硬件无关的源码 + 测试替身（stub tusb），
# 不链接 pico-sdk，用宿主 g++ 直接运行。
add_executable(pico_all_tests
    ../src/key_matrix.cpp
    ../src/key_scan.cpp
    ../src/usb_hid.cpp
    stubs/tusb_stub.cpp
    test_main.cpp
    test_matrix.cpp
    test_scan.cpp
    test_usb_hid.cpp
)

target_include_directories(pico_all_tests PRIVATE
    ../include
    stubs
)
