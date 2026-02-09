/*
 * USB Descriptors for N64-USB Gamepad
 * Dual gamepad support - USB composite device
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>

//--------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------
#define MAX_CONTROLLERS     2           // Maximum number of controllers

//--------------------------------------------------------------------
// USB IDs
//--------------------------------------------------------------------
// Using pid.codes open source VID (https://pid.codes/)
#define USB_VID             0x1209      // pid.codes VID
#define USB_PID             0x6E34      // "n4" in hex (N64)

//--------------------------------------------------------------------
// String Descriptor Indices
//--------------------------------------------------------------------
#define STRID_LANGID        0
#define STRID_MANUFACTURER  1
#define STRID_PRODUCT       2
#define STRID_SERIAL        3

//--------------------------------------------------------------------
// Interface Numbers (one HID interface per gamepad)
//--------------------------------------------------------------------
#define ITF_NUM_HID1        0
#define ITF_NUM_HID2        1
#define ITF_NUM_TOTAL       2

#endif /* USB_DESCRIPTORS_H */
