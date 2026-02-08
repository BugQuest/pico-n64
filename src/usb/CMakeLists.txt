add_library(usb_gamepad
    usb_gamepad.c
    usb_descriptors.c
)

target_link_libraries(usb_gamepad
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

target_include_directories(usb_gamepad PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
