/*
 * N64 Controller Interface
 * Functions for communicating with N64 controller via PIO
 */

#ifndef N64_CONTROLLER_H
#define N64_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"
#include "n64_protocol.h"

//--------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------
#define N64_DATA_PIN        18      // GPIO pin for N64 data line

//--------------------------------------------------------------------
// N64 Controller Handle
//--------------------------------------------------------------------
typedef struct {
    PIO pio;                // PIO instance (pio0 or pio1)
    uint sm;                // State machine number
    uint offset;            // PIO program offset
    uint pin;               // Data GPIO pin
    bool connected;         // Controller connection status
} n64_controller_t;

//--------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------

/**
 * Initialize N64 controller communication
 * @param controller Pointer to controller handle
 * @param pin GPIO pin connected to N64 data line
 * @return true if initialization successful
 */
bool n64_init(n64_controller_t *controller, uint pin);

/**
 * Read current state from N64 controller
 * @param controller Pointer to controller handle
 * @param state Pointer to state structure to fill
 * @return true if read successful, false if controller disconnected
 */
bool n64_read(n64_controller_t *controller, n64_state_t *state);

/**
 * Send command to N64 controller and receive response
 * @param controller Pointer to controller handle
 * @param cmd Command byte to send
 * @param response Buffer for response bytes
 * @param response_len Expected response length
 * @return true if communication successful
 */
bool n64_transfer(n64_controller_t *controller, uint8_t cmd,
                  uint8_t *response, uint response_len);

#endif /* N64_CONTROLLER_H */
