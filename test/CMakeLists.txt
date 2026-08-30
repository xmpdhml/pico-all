cmake_minimum_required(VERSION 3.13)
project(pico_all_tests CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Host unit tests: compile hardware-independent sources + a test stand-in
# (stub tusb), no pico-sdk link; run directly with the host g++.
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

# Disable debug logging in host tests (DEBUG_LOG expands to nothing) to avoid noise
target_compile_definitions(pico_all_tests PRIVATE DEBUG_LOG_ENABLE=0)
