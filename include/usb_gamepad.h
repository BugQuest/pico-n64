/*
 * USB HID Gamepad Interface
 * Converts N64 controller state to USB HID gamepad report
 * Supports dual controllers via separate HID interfaces
 */

#ifndef USB_GAMEPAD_H
#define USB_GAMEPAD_H

#include <stdint.h>
#include <stdbool.h>
#include "n64_protocol.h"
#include "usb_descriptors.h"

//--------------------------------------------------------------------
// USB HID Gamepad Report Structure
// No Report ID - each gamepad has its own HID interface
//--------------------------------------------------------------------
typedef struct __attribute__((packed)) {
    uint16_t buttons;       // 16 buttons (bits 0-15)
    uint8_t  hat;           // D-Pad as 8-way hat switch (0-7, 8=centered)
    uint8_t  lx;            // Left stick X (0-255, 128=center)
    uint8_t  ly;            // Left stick Y (0-255, 128=center)
} usb_gamepad_report_t;

//--------------------------------------------------------------------
// Button Bit Positions in USB Report
//--------------------------------------------------------------------
#define USB_BTN_A           (1 << 0)    // Button 1 - N64 A
#define USB_BTN_B           (1 << 1)    // Button 2 - N64 B
#define USB_BTN_Z           (1 << 2)    // Button 3 - N64 Z (trigger)
#define USB_BTN_C_UP        (1 << 3)    // Button 4 - N64 C-Up
#define USB_BTN_L           (1 << 4)    // Button 5 - N64 L
#define USB_BTN_R           (1 << 5)    // Button 6 - N64 R
#define USB_BTN_C_DOWN      (1 << 6)    // Button 7 - N64 C-Down
#define USB_BTN_C_LEFT      (1 << 7)    // Button 8 - N64 C-Left
#define USB_BTN_C_RIGHT     (1 << 8)    // Button 9 - N64 C-Right
#define USB_BTN_START       (1 << 9)    // Button 10 - N64 Start

//--------------------------------------------------------------------
// Hat Switch Values (D-Pad directions)
//--------------------------------------------------------------------
#define HAT_UP              0
#define HAT_UP_RIGHT        1
#define HAT_RIGHT           2
#define HAT_DOWN_RIGHT      3
#define HAT_DOWN            4
#define HAT_DOWN_LEFT       5
#define HAT_LEFT            6
#define HAT_UP_LEFT         7
#define HAT_CENTER          8           // Null state (no direction)

//--------------------------------------------------------------------
// Joystick Constants
//--------------------------------------------------------------------
#define JOYSTICK_CENTER     128         // USB center value (8-bit)
#define JOYSTICK_MIN        0           // USB minimum value
#define JOYSTICK_MAX        255         // USB maximum value

//--------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------

/**
 * Convert N64 controller state to USB HID gamepad report
 * @param n64 Pointer to N64 controller state
 * @param usb Pointer to USB report structure to fill
 */
void n64_to_usb_report(const n64_state_t *n64, usb_gamepad_report_t *usb);

/**
 * Initialize a neutral (centered, no buttons) report
 * @param usb Pointer to USB report structure to fill
 */
void usb_gamepad_init_neutral(usb_gamepad_report_t *usb);

/**
 * Scale N64 joystick axis value to USB HID range
 * @param n64_value N64 axis value (-80 to +80)
 * @return USB axis value (0 to 255)
 */
uint8_t scale_n64_axis(int8_t n64_value);

/**
 * Convert N64 D-Pad bits to USB hat switch value
 * @param dpad N64 D-Pad bits (4 bits)
 * @return USB hat switch value (0-8)
 */
uint8_t map_dpad_to_hat(uint8_t dpad);

/**
 * Send USB HID gamepad report on a specific HID instance
 * @param instance HID instance index (0 = P1, 1 = P2)
 * @param report Pointer to report to send
 * @return true if report sent successfully
 */
bool usb_gamepad_send_report(uint8_t instance, const usb_gamepad_report_t *report);

#endif /* USB_GAMEPAD_H */
