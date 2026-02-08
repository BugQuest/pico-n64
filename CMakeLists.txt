cmake_minimum_required(VERSION 3.13)

# Include Pico SDK - Set PICO_SDK_PATH environment variable before building
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(pico_n64 C CXX ASM)

pico_sdk_init()

add_subdirectory(src)
