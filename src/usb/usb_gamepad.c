/*
 * USB HID Gamepad Implementation
 * Converts N64 controller state to USB HID gamepad report
 * Supports dual controllers via separate HID interfaces
 */

#include "usb_gamepad.h"
#include "n64_protocol.h"
#include "tusb.h"
#include <string.h>

//--------------------------------------------------------------------
// Hat Switch Lookup Table
// Maps N64 D-Pad bit combinations to USB hat switch values
//--------------------------------------------------------------------
static const uint8_t dpad_to_hat[16] = {
    HAT_CENTER,      // 0b0000 - nothing
    HAT_RIGHT,       // 0b0001 - right
    HAT_LEFT,        // 0b0010 - left
    HAT_CENTER,      // 0b0011 - left+right (invalid, center)
    HAT_DOWN,        // 0b0100 - down
    HAT_DOWN_RIGHT,  // 0b0101 - down+right
    HAT_DOWN_LEFT,   // 0b0110 - down+left
    HAT_DOWN,        // 0b0111 - down+left+right (invalid, down)
    HAT_UP,          // 0b1000 - up
    HAT_UP_RIGHT,    // 0b1001 - up+right
    HAT_UP_LEFT,     // 0b1010 - up+left
    HAT_UP,          // 0b1011 - up+left+right (invalid, up)
    HAT_CENTER,      // 0b1100 - up+down (invalid, center)
    HAT_RIGHT,       // 0b1101 - up+down+right (invalid, right)
    HAT_LEFT,        // 0b1110 - up+down+left (invalid, left)
    HAT_CENTER       // 0b1111 - all (invalid, center)
};

//--------------------------------------------------------------------
// Public Functions
//--------------------------------------------------------------------

uint8_t scale_n64_axis(int8_t n64_value) {
    // N64 typical range: -80 to +80
    // USB HID range: 0 to 255 (128 = center)

    // Clamp to realistic N64 range
    int16_t clamped = n64_value;
    if (clamped > N64_JOYSTICK_MAX) {
        clamped = N64_JOYSTICK_MAX;
    }
    if (clamped < -N64_JOYSTICK_MAX) {
        clamped = -N64_JOYSTICK_MAX;
    }

    // Scale from (-80 to +80) to (0 to 255)
    // Formula: ((value + 80) * 255) / 160
    int32_t scaled = ((clamped + N64_JOYSTICK_MAX) * 255) / (N64_JOYSTICK_MAX * 2);

    return (uint8_t)scaled;
}

uint8_t map_dpad_to_hat(uint8_t dpad) {
    // D-Pad bits are in lower 4 bits
    return dpad_to_hat[dpad & 0x0F];
}

void usb_gamepad_init_neutral(usb_gamepad_report_t *usb) {
    usb->buttons = 0;
    usb->hat = HAT_CENTER;
    usb->lx = JOYSTICK_CENTER;
    usb->ly = JOYSTICK_CENTER;
}

void n64_to_usb_report(const n64_state_t *n64, usb_gamepad_report_t *usb) {
    // Clear buttons
    usb->buttons = 0;

    // Map buttons from byte 0 (A, B, Z, Start)
    if (n64->buttons0 & N64_MASK_A) {
        usb->buttons |= USB_BTN_A;
    }
    if (n64->buttons0 & N64_MASK_B) {
        usb->buttons |= USB_BTN_B;
    }
    if (n64->buttons0 & N64_MASK_Z) {
        usb->buttons |= USB_BTN_Z;
    }
    if (n64->buttons0 & N64_MASK_START) {
        usb->buttons |= USB_BTN_START;
    }

    // Map buttons from byte 1 (L, R, C-buttons)
    if (n64->buttons1 & N64_MASK_L) {
        usb->buttons |= USB_BTN_L;
    }
    if (n64->buttons1 & N64_MASK_R) {
        usb->buttons |= USB_BTN_R;
    }

    // Map C-buttons as separate buttons
    uint8_t c_buttons = n64->buttons1 & N64_MASK_C;
    if (c_buttons & N64_C_UP) {
        usb->buttons |= USB_BTN_C_UP;
    }
    if (c_buttons & N64_C_DOWN) {
        usb->buttons |= USB_BTN_C_DOWN;
    }
    if (c_buttons & N64_C_LEFT) {
        usb->buttons |= USB_BTN_C_LEFT;
    }
    if (c_buttons & N64_C_RIGHT) {
        usb->buttons |= USB_BTN_C_RIGHT;
    }

    // Map D-Pad to hat switch
    usb->hat = map_dpad_to_hat(n64->buttons0 & N64_MASK_DPAD);

    // Map analog stick (invert Y axis for standard USB convention)
    usb->lx = scale_n64_axis(n64->stick_x);
    usb->ly = 255 - scale_n64_axis(n64->stick_y);  // Invert Y
}

bool usb_gamepad_send_report(uint8_t instance, const usb_gamepad_report_t *report) {
    // Only send if this HID instance is ready
    if (!tud_hid_n_ready(instance)) {
        return false;
    }

    // Send HID report on specific instance (report_id=0, no Report ID prefix)
    return tud_hid_n_report(instance, 0, report, sizeof(usb_gamepad_report_t));
}

//--------------------------------------------------------------------
// TinyUSB HID Callbacks
//--------------------------------------------------------------------

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t *buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    // Not used for gamepad
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint (Report ID = 0, Type = 0)
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;

    // Could be used for rumble in the future
}
