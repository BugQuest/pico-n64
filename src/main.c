/*
 * N64-USB Dual Gamepad Adapter
 * Main Application for Raspberry Pi Pico
 *
 * Converts up to 2 N64 controllers to USB HID gamepads
 * Dynamically detects 0, 1, or 2 connected controllers
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"

#include "n64_controller.h"
#include "n64_protocol.h"
#include "usb_gamepad.h"
#include "usb_descriptors.h"

//--------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------
#define LED_PIN             PICO_DEFAULT_LED_PIN    // Built-in LED (GP25)
#define POLL_INTERVAL_MS    8                        // ~125Hz polling rate

//--------------------------------------------------------------------
// LED Status Patterns
//--------------------------------------------------------------------
typedef enum {
    LED_OFF,                // USB not connected
    LED_BLINK_SLOW,         // No controllers connected
    LED_BLINK_MEDIUM,       // 1 controller connected
    LED_ON                  // 2 controllers connected (or error if fast)
} led_status_t;

//--------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------
static n64_controller_t g_controllers[MAX_CONTROLLERS];
static n64_state_t g_states[MAX_CONTROLLERS];
static usb_gamepad_report_t g_reports[MAX_CONTROLLERS];
static led_status_t g_led_status = LED_OFF;
static uint32_t g_last_led_toggle = 0;
static bool g_led_state = false;
static bool g_pio_init_ok = false;

// External LED states
static bool g_ext_leds_enabled[MAX_CONTROLLERS] = {false, false};

// Connection tracking for debug logs
static bool g_was_connected[MAX_CONTROLLERS] = {false, false};
static uint32_t g_connect_count[MAX_CONTROLLERS] = {0, 0};

//--------------------------------------------------------------------
// External LED Management (optional per-controller LEDs)
//--------------------------------------------------------------------
static void init_external_leds(void) {
    for (int i = 0; i < MAX_CONTROLLERS; i++) {
        uint pin = N64_LED_PINS[i];
        if (pin != 0) {
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, false);
            g_ext_leds_enabled[i] = true;
            printf("  External LED %d on GP%d: OK\n", i + 1, pin);
        }
    }
}

static void update_external_leds(void) {
    for (int i = 0; i < MAX_CONTROLLERS; i++) {
        if (g_ext_leds_enabled[i]) {
            // LED ON when controller is connected, OFF otherwise
            gpio_put(N64_LED_PINS[i], g_controllers[i].connected);
        }
    }
}

//--------------------------------------------------------------------
// Built-in LED Management
//--------------------------------------------------------------------
static void update_led(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (g_led_status) {
        case LED_OFF:
            gpio_put(LED_PIN, false);
            break;

        case LED_BLINK_SLOW:
            // No controllers - slow blink (1000ms)
            if (now - g_last_led_toggle > 1000) {
                g_led_state = !g_led_state;
                gpio_put(LED_PIN, g_led_state);
                g_last_led_toggle = now;
            }
            break;

        case LED_BLINK_MEDIUM:
            // 1 controller - medium blink (300ms)
            if (now - g_last_led_toggle > 300) {
                g_led_state = !g_led_state;
                gpio_put(LED_PIN, g_led_state);
                g_last_led_toggle = now;
            }
            break;

        case LED_ON:
            // 2 controllers - solid on
            gpio_put(LED_PIN, true);
            break;
    }
}

//--------------------------------------------------------------------
// Count connected controllers
//--------------------------------------------------------------------
static uint8_t count_connected(void) {
    uint8_t count = 0;
    for (int i = 0; i < MAX_CONTROLLERS; i++) {
        if (g_controllers[i].connected) {
            count++;
        }
    }
    return count;
}

//--------------------------------------------------------------------
// Update LED based on connection status
//--------------------------------------------------------------------
static void update_led_status(void) {
    if (!tud_mounted()) {
        g_led_status = LED_OFF;
        return;
    }

    if (!g_pio_init_ok) {
        // Fast blink for error (reuse BLINK_SLOW with shorter toggle in update)
        g_led_status = LED_BLINK_SLOW;
        return;
    }

    uint8_t connected = count_connected();
    switch (connected) {
        case 0:
            g_led_status = LED_BLINK_SLOW;
            break;
        case 1:
            g_led_status = LED_BLINK_MEDIUM;
            break;
        default:
            g_led_status = LED_ON;
            break;
    }
}

//--------------------------------------------------------------------
// Main Application
//--------------------------------------------------------------------
int main(void) {
    // Initialize standard I/O (UART for debug)
    stdio_init_all();

    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    // Initialize TinyUSB
    tusb_init();

    // Initialize N64 controllers
    printf("N64-USB Dual Gamepad Adapter\n");
    printf("Initializing %d controller ports...\n", MAX_CONTROLLERS);

    g_pio_init_ok = true;
    for (int i = 0; i < MAX_CONTROLLERS; i++) {
        printf("  Controller %d on GP%d: ", i + 1, N64_DATA_PINS[i]);

        if (n64_init(&g_controllers[i], N64_DATA_PINS[i])) {
            printf("OK\n");
        } else {
            printf("FAILED (PIO unavailable)\n");
            g_pio_init_ok = false;
        }

        // Initialize neutral reports
        usb_gamepad_init_neutral(&g_reports[i]);
    }

    if (!g_pio_init_ok) {
        printf("ERROR: Not all controllers could be initialized\n");
    }

    // Initialize optional external LEDs
    printf("Initializing external LEDs...\n");
    init_external_leds();

    printf("Waiting for controllers...\n");
    g_led_status = LED_BLINK_SLOW;

    // Main loop
    uint32_t last_poll = 0;

    while (true) {
        // Process USB tasks
        tud_task();

        // Update LED
        update_led();

        // Poll controllers at fixed interval
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_poll < POLL_INTERVAL_MS) {
            continue;
        }
        last_poll = now;

        // Check USB connection status
        if (!tud_mounted()) {
            g_led_status = LED_OFF;
            continue;
        }

        // Read and send reports only for connected controllers
        for (int i = 0; i < MAX_CONTROLLERS; i++) {
            bool responding = n64_read(&g_controllers[i], &g_states[i]);

            // Detect connection state changes
            if (responding && !g_was_connected[i]) {
                g_connect_count[i]++;
                if (g_connect_count[i] == 1) {
                    printf("[P%d] Connected (GP%d)\n", i + 1, N64_DATA_PINS[i]);
                } else {
                    printf("[P%d] Reconnected (GP%d) - #%lu\n",
                           i + 1, N64_DATA_PINS[i], g_connect_count[i]);
                }
            } else if (!responding && g_was_connected[i]) {
                printf("[P%d] Disconnected (GP%d)\n", i + 1, N64_DATA_PINS[i]);
                // Send one final neutral report so the host sees all buttons released
                usb_gamepad_init_neutral(&g_reports[i]);
                usb_gamepad_send_report(i, &g_reports[i]);
            }
            g_was_connected[i] = responding;

            if (responding) {
                n64_to_usb_report(&g_states[i], &g_reports[i]);
                usb_gamepad_send_report(i, &g_reports[i]);
            }
        }

        // Update LED status based on connected controllers
        update_led_status();
        update_external_leds();
    }

    return 0;
}
