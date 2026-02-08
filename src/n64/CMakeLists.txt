add_library(n64_controller
    n64_controller.c
)

target_link_libraries(n64_controller
    pico_stdlib
    hardware_pio
)

target_include_directories(n64_controller PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Generate PIO header from .pio file
pico_generate_pio_header(n64_controller
    ${CMAKE_CURRENT_LIST_DIR}/n64_controller.pio
)
