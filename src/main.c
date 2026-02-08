/*
 * N64-USB Gamepad Adapter
 * Main Application for Raspberry Pi Pico
 *
 * Converts N64 controller input to USB HID gamepad
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"

#include "n64_controller.h"
#include "n64_protocol.h"
#include "usb_gamepad.h"

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
    LED_BLINK_SLOW,         // Waiting for N64 controller
    LED_BLINK_FAST,         // Error
    LED_ON                  // Normal operation
} led_status_t;

//--------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------
static n64_controller_t g_controller;
static n64_state_t g_state;
static usb_gamepad_report_t g_report;
static led_status_t g_led_status = LED_OFF;
static uint32_t g_last_led_toggle = 0;
static bool g_led_state = false;

//--------------------------------------------------------------------
// LED Management
//--------------------------------------------------------------------
static void update_led(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (g_led_status) {
        case LED_OFF:
            gpio_put(LED_PIN, false);
            break;

        case LED_BLINK_SLOW:
            if (now - g_last_led_toggle > 500) {
                g_led_state = !g_led_state;
                gpio_put(LED_PIN, g_led_state);
                g_last_led_toggle = now;
            }
            break;

        case LED_BLINK_FAST:
            if (now - g_last_led_toggle > 100) {
                g_led_state = !g_led_state;
                gpio_put(LED_PIN, g_led_state);
                g_last_led_toggle = now;
            }
            break;

        case LED_ON:
            gpio_put(LED_PIN, true);
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

    // Initialize N64 controller
    printf("N64-USB Gamepad Adapter\n");
    printf("Initializing N64 controller on GP%d...\n", N64_DATA_PIN);

    if (!n64_init(&g_controller, N64_DATA_PIN)) {
        printf("ERROR: Failed to initialize PIO for N64 controller\n");
        g_led_status = LED_BLINK_FAST;
        while (1) {
            update_led();
            sleep_ms(10);
        }
    }

    printf("N64 controller initialized\n");
    g_led_status = LED_BLINK_SLOW;

    // Main loop
    uint32_t last_poll = 0;

    while (true) {
        // Process USB tasks
        tud_task();

        // Update LED
        update_led();

        // Poll controller at fixed interval
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

        // Read N64 controller
        if (n64_read(&g_controller, &g_state)) {
            // Controller connected and responding
            g_led_status = LED_ON;

            // Convert to USB report
            n64_to_usb_report(&g_state, &g_report);

            // Send USB HID report
            usb_gamepad_send_report(&g_report);
        } else {
            // Controller not responding
            g_led_status = LED_BLINK_SLOW;

            // Send neutral report (all buttons released, stick centered)
            g_report.buttons = 0;
            g_report.hat = HAT_CENTER;
            g_report.lx = JOYSTICK_CENTER;
            g_report.ly = JOYSTICK_CENTER;
            usb_gamepad_send_report(&g_report);
        }
    }

    return 0;
}
