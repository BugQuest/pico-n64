/*
 * USB Descriptors for N64-USB Gamepad
 * Standard USB HID gamepad compatible with Windows/Linux/macOS
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>

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
// HID Report ID (not used, single report)
//--------------------------------------------------------------------
#define REPORT_ID_GAMEPAD   0

//--------------------------------------------------------------------
// External Declarations (defined in usb_descriptors.c)
//--------------------------------------------------------------------
extern const uint8_t hid_report_descriptor[];
extern const uint16_t hid_report_descriptor_len;

#endif /* USB_DESCRIPTORS_H */
