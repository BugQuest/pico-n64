add_subdirectory(n64)
add_subdirectory(usb)

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_pio
    n64_controller
    usb_gamepad
    tinyusb_device
    tinyusb_board
)

# Disable USB stdio (we use USB for HID)
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# Generate UF2 output file
pico_add_extra_outputs(${PROJECT_NAME})
