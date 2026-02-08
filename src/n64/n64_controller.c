/*
 * N64 Controller Implementation
 * Handles communication with N64 controller via PIO
 */

#include "n64_controller.h"
#include "n64_controller.pio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include <string.h>

//--------------------------------------------------------------------
// Private Function Declarations
//--------------------------------------------------------------------
static void send_request(PIO pio, uint sm, const uint8_t *request, uint8_t length);
static bool get_response(PIO pio, uint sm, uint8_t *response, uint8_t length);
static void reset_state_machine(n64_controller_t *controller);

//--------------------------------------------------------------------
// Public Functions
//--------------------------------------------------------------------

bool n64_init(n64_controller_t *controller, uint pin) {
    // Try to add program to PIO0 first, then PIO1
    PIO pio_instances[] = {pio0, pio1};
    PIO selected_pio = NULL;
    uint offset = 0;

    for (int i = 0; i < 2; i++) {
        if (pio_can_add_program(pio_instances[i], &n64_controller_program)) {
            selected_pio = pio_instances[i];
            break;
        }
    }

    if (selected_pio == NULL) {
        return false;  // No PIO available
    }

    // Add program and claim state machine
    offset = pio_add_program(selected_pio, &n64_controller_program);
    int sm = pio_claim_unused_sm(selected_pio, false);
    if (sm < 0) {
        return false;  // No state machine available
    }

    // Store controller configuration
    controller->pio = selected_pio;
    controller->sm = (uint)sm;
    controller->offset = offset;
    controller->pin = pin;
    controller->connected = false;

    // Initialize PIO state machine
    pio_sm_config c = n64_controller_program_get_default_config(offset);
    n64_controller_program_init(selected_pio, controller->sm, offset, pin, &c);

    // Test connection by sending info command
    uint8_t response[3];
    if (n64_transfer(controller, N64_CMD_INFO, response, 3)) {
        // Check if response indicates N64 controller (0x05 for standard controller)
        controller->connected = (response[0] == 0x05);
    }

    return true;
}

bool n64_read(n64_controller_t *controller, n64_state_t *state) {
    uint8_t response[N64_STATUS_SIZE];

    if (!n64_transfer(controller, N64_CMD_STATUS, response, N64_STATUS_SIZE)) {
        controller->connected = false;
        return false;
    }

    controller->connected = true;

    // Parse response into state structure
    state->buttons0 = response[0];
    state->buttons1 = response[1];
    state->stick_x = (int8_t)response[2];
    state->stick_y = (int8_t)response[3];

    return true;
}

bool n64_transfer(n64_controller_t *controller, uint8_t cmd,
                  uint8_t *response, uint response_len) {
    PIO pio = controller->pio;
    uint sm = controller->sm;

    // Reset state machine to ensure clean state
    reset_state_machine(controller);

    // Send response length (minus 1, as expected by PIO program)
    // The PIO program expects the count in the upper 8 bits
    pio_sm_put_blocking(pio, sm, ((response_len - 1) & 0x1F) << 24);

    // Send command byte
    uint8_t request[] = {cmd};
    send_request(pio, sm, request, 1);

    // Get response
    if (!get_response(pio, sm, response, response_len)) {
        // Reset state machine on failure to recover from stuck state
        reset_state_machine(controller);
        return false;
    }

    // Wait for communication to complete
    // 4us per byte plus settling time
    busy_wait_us(4 * (1 + response_len) + 450);

    return true;
}

//--------------------------------------------------------------------
// Private Functions
//--------------------------------------------------------------------

static void send_request(PIO pio, uint sm, const uint8_t *request, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        // PIO expects data in upper 8 bits of 32-bit word
        pio_sm_put_blocking(pio, sm, ((uint32_t)request[i]) << 24);
    }
}

static bool get_response(PIO pio, uint sm, uint8_t *response, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        // Wait for response with timeout
        absolute_time_t timeout = make_timeout_time_us(600);
        bool timed_out = false;

        while (pio_sm_is_rx_fifo_empty(pio, sm) && !timed_out) {
            timed_out = time_reached(timeout);
        }

        if (timed_out) {
            return false;  // Controller not responding
        }

        // Read response byte (lower 8 bits of 32-bit word)
        uint32_t data = pio_sm_get(pio, sm);
        response[i] = (uint8_t)(data & 0xFF);
    }

    return true;
}

static void reset_state_machine(n64_controller_t *controller) {
    PIO pio = controller->pio;
    uint sm = controller->sm;
    uint offset = controller->offset;

    // Disable state machine
    pio_sm_set_enabled(pio, sm, false);

    // Clear FIFOs
    pio_sm_clear_fifos(pio, sm);

    // Reset state machine to beginning of program
    pio_sm_restart(pio, sm);
    pio_sm_exec(pio, sm, pio_encode_jmp(offset));

    // Re-enable state machine
    pio_sm_set_enabled(pio, sm, true);
}
