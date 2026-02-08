/*
 * USB Descriptors Implementation for N64-USB Gamepad
 * Standard USB HID gamepad compatible with Windows/Linux/macOS
 */

#include "usb_descriptors.h"
#include "tusb.h"

//--------------------------------------------------------------------
// HID Report Descriptor
// Standard gamepad with 10 buttons, hat switch, and 2 analog axes
//--------------------------------------------------------------------
const uint8_t hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)

    // 16 Buttons (only first 10 used)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x10,        //   Usage Maximum (Button 16)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data, Var, Abs)

    // Hat Switch (D-Pad) - 4 bits
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x39,        //   Usage (Hat Switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (English Rotation: Degree)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data, Var, Abs, Null State)

    // Padding - 4 bits
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const, Var, Abs)

    // Left Stick X and Y - 2 axes, 8-bit each
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data, Var, Abs)

    0xC0               // End Collection
};

const uint16_t hid_report_descriptor_len = sizeof(hid_report_descriptor);

//--------------------------------------------------------------------
// Device Descriptor
//--------------------------------------------------------------------
static const tusb_desc_device_t device_descriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,               // USB 2.0
    .bDeviceClass       = 0x00,                 // Defined in interface
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,               // Version 1.0
    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,
    .bNumConfigurations = 0x01
};

//--------------------------------------------------------------------
// Configuration Descriptor
//--------------------------------------------------------------------
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID         0x81

static const uint8_t config_descriptor[] = {
    // Configuration descriptor
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // HID Interface descriptor
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_descriptor), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 8)
};

//--------------------------------------------------------------------
// String Descriptors
//--------------------------------------------------------------------
static const char *string_descriptors[] = {
    (const char[]){0x09, 0x04},     // 0: Language (English US)
    "N64-USB",                       // 1: Manufacturer
    "N64 Controller Adapter",        // 2: Product
    "0001",                          // 3: Serial Number
};

//--------------------------------------------------------------------
// TinyUSB Callbacks
//--------------------------------------------------------------------

// Invoked when host requests device descriptor
const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&device_descriptor;
}

// Invoked when host requests configuration descriptor
const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;  // Only one configuration
    return config_descriptor;
}

// Invoked when host requests HID report descriptor
const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return hid_report_descriptor;
}

// Invoked when host requests string descriptor
const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t str_desc[32];

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&str_desc[1], string_descriptors[0], 2);
        chr_count = 1;
    } else {
        if (index >= sizeof(string_descriptors) / sizeof(string_descriptors[0])) {
            return NULL;
        }

        const char *str = string_descriptors[index];
        chr_count = (uint8_t)strlen(str);
        if (chr_count > 31) {
            chr_count = 31;
        }

        // Convert ASCII to UTF-16
        for (uint8_t i = 0; i < chr_count; i++) {
            str_desc[1 + i] = str[i];
        }
    }

    // First byte is length (including header), second byte is descriptor type
    str_desc[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return str_desc;
}
