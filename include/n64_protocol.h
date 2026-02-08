/*
 * N64 Controller Protocol Constants
 * Bit masks and command definitions for Nintendo 64 controller communication
 */

#ifndef N64_PROTOCOL_H
#define N64_PROTOCOL_H

#include <stdint.h>

//--------------------------------------------------------------------
// N64 Commands
//--------------------------------------------------------------------
#define N64_CMD_INFO        0x00    // Get controller info
#define N64_CMD_STATUS      0x01    // Get button/joystick status (returns 4 bytes)
#define N64_CMD_READ        0x02    // Read from controller pak
#define N64_CMD_WRITE       0x03    // Write to controller pak

//--------------------------------------------------------------------
// N64 Controller Response Size
//--------------------------------------------------------------------
#define N64_STATUS_SIZE     4       // 4 bytes response for status command

//--------------------------------------------------------------------
// Byte 0 - Buttons (bits 7-4) and D-Pad (bits 3-0)
//--------------------------------------------------------------------
#define N64_MASK_A          (1 << 7)    // A button
#define N64_MASK_B          (1 << 6)    // B button
#define N64_MASK_Z          (1 << 5)    // Z trigger
#define N64_MASK_START      (1 << 4)    // Start button
#define N64_MASK_DPAD       0x0F        // D-Pad mask (bits 0-3)

// D-Pad values (active low logic, combined)
#define N64_DPAD_UP         0x08
#define N64_DPAD_DOWN       0x04
#define N64_DPAD_LEFT       0x02
#define N64_DPAD_RIGHT      0x01

//--------------------------------------------------------------------
// Byte 1 - Shoulder buttons and C-Pad
//--------------------------------------------------------------------
#define N64_MASK_RESET      (1 << 7)    // Reset (unused)
#define N64_MASK_L          (1 << 5)    // L shoulder
#define N64_MASK_R          (1 << 4)    // R shoulder
#define N64_MASK_C          0x0F        // C-buttons mask (bits 0-3)

// C-button values
#define N64_C_UP            0x08
#define N64_C_DOWN          0x04
#define N64_C_LEFT          0x02
#define N64_C_RIGHT         0x01

//--------------------------------------------------------------------
// Bytes 2-3 - Analog Joystick
//--------------------------------------------------------------------
// Byte 2: X-axis (signed 8-bit, typical range -80 to +80)
// Byte 3: Y-axis (signed 8-bit, typical range -80 to +80)
#define N64_JOYSTICK_MAX    80          // Typical maximum absolute value
#define N64_JOYSTICK_CENTER 0           // Center position

//--------------------------------------------------------------------
// N64 Controller State Structure
//--------------------------------------------------------------------
typedef struct {
    uint8_t buttons0;       // A, B, Z, Start, D-Pad
    uint8_t buttons1;       // Reset, L, R, C-buttons
    int8_t  stick_x;        // Analog stick X (-80 to +80)
    int8_t  stick_y;        // Analog stick Y (-80 to +80)
} n64_state_t;

#endif /* N64_PROTOCOL_H */
