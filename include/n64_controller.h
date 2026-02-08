/*
 * N64 Controller Interface
 * Functions for communicating with N64 controller via PIO
 * Supports dual controllers
 */

#ifndef N64_CONTROLLER_H
#define N64_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"
#include "n64_protocol.h"
#include "usb_descriptors.h"

//--------------------------------------------------------------------
// Configuration - GPIO Pins for N64 data lines
//--------------------------------------------------------------------
#define N64_DATA_PIN_1      18      // GPIO pin for controller 1
#define N64_DATA_PIN_2      19      // GPIO pin for controller 2

// Array of pins for easy iteration
static const uint N64_DATA_PINS[MAX_CONTROLLERS] = {
    N64_DATA_PIN_1,
    N64_DATA_PIN_2
};

//--------------------------------------------------------------------
// Configuration - Optional External Status LEDs
// Set to 0 to disable, or GPIO pin number to enable
// LEDs are active HIGH (on when controller connected)
//--------------------------------------------------------------------
#define N64_LED_PIN_1       16      // External LED for controller 1 (0 = disabled)
#define N64_LED_PIN_2       17      // External LED for controller 2 (0 = disabled)

// Array of LED pins for easy iteration
static const uint N64_LED_PINS[MAX_CONTROLLERS] = {
    N64_LED_PIN_1,
    N64_LED_PIN_2
};

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
