/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include <rf.h>
#include "rf_device.h"
#include "CH59x_common.h"
#include "usb.h"
#include "ota.h"
#include "trans.h"
#include "mouse.h"
#include "peripheral.h"
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
// 支持的最大接口数量
#define USB_INTERFACE_MAX_NUM       4
// 接口号的最大值
#define USB_INTERFACE_MAX_INDEX     3

const uint8_t LangID_StrDescr[ 4 ] =
{
    0x04,
    0x03,                                                                       /* bDescriptorType */
    0x09,
    0x04
};

// 厂家信息
uint8_t MyManuInfo[64] = {0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0};

// 产品信息
uint8_t MyProdInfo[64] = {0x0D+11, 0x03,
    '2', 0,
    '.', 0,
    '4', 0,
    'G', 0,
    ' ', 0,
    'D', 0,
    'o', 0,
    'n', 0,
    'g', 0,
    'l', 0,
    'e', 0
};

// 序列号
uint8_t MySerialNum[64] = {0x0D+11, 0x03,
    '2', 0,
    '.', 0,
    '4', 0,
    'G', 0,
    ' ', 0,
    'D', 0,
    'o', 0,
    'n', 0,
    'g', 0,
    'l', 0,
    'e', 0
};


//   -----报告描述符Mouse--------------
const uint8_t HID_ReportDescriptorMouse[]=
{
        0x05,0x01,                  //81    GLOBAL_USAGE_PAGE(Generic Desktop Controls)
        0x09,0x02,                  //83    LOCAL_USAGE(Mouse)
        0xA1,0x01,                  //85    MAIN_COLLECTION(Applicatior)
            0x09,0x01,                  //89    LOCAL_USAGE(Pointer)
            0xA1,0x00,                  //91    MAIN_COLLECTION(Physical)
                0x05,0x09,                  //93    GLOBAL_USAGE_PAGE(Button)
                0x19,0x01,                  //95    LOCAL_USAGE_MINIMUM(1)
                0x29,0x05,                  //97    LOCAL_USAGE_MAXIMUM(5)
                0x15,0x00,                  //99    GLOBAL_LOGICAL_MINIMUM(0)
                0x25,0x01,                  //101   GLOBAL_LOCAL_MAXIMUM(1)
                0x95,0x05,                  //103   GLOBAL_REPORT_COUNT(5)
                0x75,0x01,                  //105   GLOBAL_REPORT_SIZE(1)
                0x81,0x02,                  //107   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 18.5
                0x95,0x01,                  //109   GLOBAL_REPORT_COUNT(1)
                0x75,0x03,                  //111   GLOBAL_REPORT_SIZE(3)
                0x81,0x01,                  //113   MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )   Input 19.0
                0x05,0x01,                  //115   GLOBAL_USAGE_PAGE(Generic Desktop Controls)
                0x09,0x30,                  //117   LOCAL_USAGE(X)
                0x09,0x31,                  //119   LOCAL_USAGE(Y)
    //            0x16, 0x00, 0x80,  //     Logical Minimum (-32768)
    //            0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    //            0x75,0x10,                  //127   GLOBAL_REPORT_SIZE(16)
                0x15, 0x80,  //     Logical Minimum (-128)
                0x25, 0x7F,  //     Logical Maximum (128)
                0x75,0x08,                  //127   GLOBAL_REPORT_SIZE(16)
                0x95,0x02,                  //129   GLOBAL_REPORT_COUNT(2)
                0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
                0x09,0x38,                  //121   LOCAL_USAGE(Wheel)
                0x15,0x81,                  //123   GLOBAL_LOGICAL_MINIMUM(-127)
                0x25,0x7F,                  //125   GLOBAL_LOCAL_MAXIMUM(127)
                0x75,0x08,                  //127   GLOBAL_REPORT_SIZE(8)
                0x95,0x01,                  //129   GLOBAL_REPORT_COUNT(1)
                0x81,0x06,                  //131   MAIN_INPUT(data var relative NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 22.0
            0xC0,                       //133   MAIN_COLLECTION_END
        0xC0,

};

//   -----报告描述符Other--------------
const uint8_t HID_ReportDescriptorOther[]=
{
    //键盘报表：
//      0x05, 0x01,     // Usage Pg (Generic Desktop)
//      0x09, 0x06,     // Usage (Keyboard)
//      0xA1, 0x01,     // Collection: (Application)
//      0x85, 0x01,     //   ReportID(1)
//                      //
//      0x05, 0x07,     // Usage Pg (Key Codes)
//      0x19, 0xE0,     // Usage Min (224)
//      0x29, 0xE7,     // Usage Max (231)
//      0x15, 0x00,     // Log Min (0)
//      0x25, 0x01,     // Log Max (1)
//                      //
//                      // Modifier byte
//      0x75, 0x01,     // Report Size (1)
//      0x95, 0x08,     // Report Count (8)
//      0x81, 0x02,     // Input: (Data, Variable, Absolute)
//                      //
//                      // Reserved byte
//      0x95, 0x01,     // Report Count (1)
//      0x75, 0x08,     // Report Size (8)
//      0x81, 0x01,     // Input: (Constant)
//                      //
//                      // LED report
//      0x95, 0x05,     // Report Count (5)
//      0x75, 0x01,     // Report Size (1)
//      0x05, 0x08,     // Usage Pg (LEDs)
//      0x19, 0x01,     // Usage Min (1)
//      0x29, 0x05,     // Usage Max (5)
//      0x91, 0x02,     // Output: (Data, Variable, Absolute)
//                      //
//                      // LED report padding
//      0x95, 0x01,     // Report Count (1)
//      0x75, 0x03,     // Report Size (3)
//      0x91, 0x01,     // Output: (Constant)
//                      //
//                      // Key arrays (6 bytes)
//      0x95, 0x06,     // Report Count (6)
//      0x75, 0x08,     // Report Size (8)
//      0x15, 0x00,     // Log Min (0)
//      0x25, 0x65,     // Log Max (101)
//      0x05, 0x07,     // Usage Pg (Key Codes)
//      0x19, 0x00,     // Usage Min (0)
//      0x29, 0x65,     // Usage Max (101)
//      0x81, 0x00,     // Input: (Data, Array)
//                      //
//      0xC0 ,           // End Collection

        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x06,        // Usage (Keyboard)
        0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,        //   Report ID (1)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0xE0,        //   Usage Minimum (0xE0)
        0x29, 0xE7,        //   Usage Maximum (0xE7)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x08,        //   Report Count (8)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x08,        //   Report Size (8)
        0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x05,        //   Report Count (5)
        0x75, 0x01,        //   Report Size (1)
        0x05, 0x08,        //   Usage Page (LEDs)
        0x19, 0x01,        //   Usage Minimum (Num Lock)
        0x29, 0x05,        //   Usage Maximum (Kana)
        0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x03,        //   Report Size (3)
        0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x06,        //   Report Count (6)
        0x75, 0x08,        //   Report Size (8)
        0x15, 0x00,        //   Logical Minimum (0)
        0x26, 0xFF, 0x00,  //   Logical Maximum (255)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0x00,        //   Usage Minimum (0x00)
        0x29, 0xFF,        //   Usage Maximum (0xFF)
        0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x0C,        //   Usage Page (Consumer)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x01,        //   Report Count (1)
        0x09, 0xB8,        //   Usage (Eject)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0xFF,        //   Usage Page (Reserved 0xFF)
        0x09, 0x03,        //   Usage (0x03)
        0x75, 0x07,        //   Report Size (7)
        0x95, 0x01,        //   Report Count (1)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              // End Collection

        // 90 bytes


    //全键盘：
    0x05,0x01,                  //0     GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x06,                  //2     LOCAL_USAGE(Keyboard)
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
    0x85,0x02,                  //6     GLOBAL_REPORT_ID(2)
    0x05,0x07,                  //8     GLOBAL_USAGE_PAGE(Keyboard/Keypad)
    0x19,0x04,                  //10    LOCAL_USAGE_MINIMUM(4)
    0x29,0x70,                  //12    LOCAL_USAGE_MAXIMUM(112)
    0x15,0x00,                  //14    GLOBAL_LOGICAL_MINIMUM(0)
    0x25,0x01,                  //16    GLOBAL_LOCAL_MAXIMUM(1)
    0x75,0x01,                  //18    GLOBAL_REPORT_SIZE(1)
    0x95,0x78,                  //20    GLOBAL_REPORT_COUNT(120)
    0x81,0x02,                  //22    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 15.0
    0xC0,                       //24    MAIN_COLLECTION_END


    //多媒体控制：
    0x05,0x0C,                  //0     GLOBAL_USAGE_PAGE(Consumer)
    0x09,0x01,                  //2     LOCAL_USAGE(    Consumer Control    )
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
    0x85,0x03,                  //6     GLOBAL_REPORT_ID(3)
    0x15,0x00,                  //8     GLOBAL_LOGICAL_MINIMUM(0)
    0x26,0xFF,0x1F,             //10    GLOBAL_LOCAL_MAXIMUM(8191/8191)
    0x19,0x00,                  //13    LOCAL_USAGE_MINIMUM(0)
    0x2A,0xFF,0x1F,             //15    LOCAL_USAGE_MAXIMUM(8191)
    0x75,0x10,                  //18    GLOBAL_REPORT_SIZE(16)
    0x95,0x01,                  //20    GLOBAL_REPORT_COUNT(1)
    0x81,0x00,                  //22    MAIN_INPUT(data array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )    Input 2.0
    0xC0,                       //24    MAIN_COLLECTION_END


    //系统控制：
    0x05,0x01,                  //0     GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x80,                  //2     LOCAL_USAGE()
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
    0x85,0x04,                  //6     GLOBAL_REPORT_ID(4)
    0x05,0x01,                  //8     GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x19,0x81,                  //10    LOCAL_USAGE_MINIMUM(129)
    0x29,0x83,                  //12    LOCAL_USAGE_MAXIMUM(131)
    0x15,0x00,                  //14    GLOBAL_LOGICAL_MINIMUM(0)
    0x25,0x01,                  //16    GLOBAL_LOCAL_MAXIMUM(1)
    0x95,0x03,                  //18    GLOBAL_REPORT_COUNT(3)
    0x75,0x01,                  //20    GLOBAL_REPORT_SIZE(1)
    0x81,0x02,                  //22    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 0.3
    0x95,0x01,                  //24    GLOBAL_REPORT_COUNT(1)
    0x75,0x05,                  //26    GLOBAL_REPORT_SIZE(5)
    0x81,0x01,                  //28    MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )   Input 1.0
    0xC0,                       //30    MAIN_COLLECTION_END
};

//   -----报告描述符Manufacturer--------------
const uint8_t HID_ReportDescriptorManufacturer[]=
{
    0x06,0x13,0xFF,             //0     GLOBAL_USAGE_PAGE(Reserved or Other)
    0x09,0x01,                  //3     LOCAL_USAGE()
    0xA1,0x01,                  //5     MAIN_COLLECTION(Applicatior)
    0x15,0x00,                  //7     GLOBAL_LOGICAL_MINIMUM(0)
    0x26,0xFF,0x00,             //9     GLOBAL_LOCAL_MAXIMUM(255/255)
    0x75,0x08,                  //12    GLOBAL_REPORT_SIZE(8)
    0x95,0x40,                  //14    GLOBAL_REPORT_COUNT(64)
    0x09,0x02,                  //16    LOCAL_USAGE()
    0x81,0x02,                  //18    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 64.0
    0x09,0x03,                  //20    LOCAL_USAGE()
    0x91,0x02,                  //22    MAIN_OUTPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile ) Output 64.0
    0x0A,0x00,0xFF,             //24    LOCAL_USAGE()
    0xB1,0x02,                  //27    MAIN_FEATURE(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )    Feature 64.0
    0xC0,                       //29    MAIN_COLLECTION_END
};

//   -----报告描述符IAP--------------
const uint8_t HID_ReportDescriptorIAP[]=
{

    0x06, 0x60, 0xFF,  // Usage Page (Vendor Defined 0xFF60)
    0x09, 0x61,        // Usage (0x61)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x62,        //   Usage (0x62)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x40,        //   Report Count (63)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x63,        //   Usage (0x63)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x40,        //   Report Count (63)
    0x75, 0x08,        //   Report Size (8)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};

const uint8_t HID_ReportDescSizeMouse = sizeof(HID_ReportDescriptorMouse);
const uint8_t HID_ReportDescSizeOther = sizeof(HID_ReportDescriptorOther);
const uint8_t HID_ReportDescSizeManufacturer = sizeof(HID_ReportDescriptorManufacturer);
const uint8_t HID_ReportDescSizeIAP = sizeof(HID_ReportDescriptorIAP);

// 设备描述符
uint8_t MyDevDescr[] = {
    0x12,   /* bLength */
    0x01,   /* bDescriptorType USB_DEVICE_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0110),  /* bcdUSB */
    0x00,   /* bDeviceClass */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    DevEP0SIZE, /* bMaxPacketSize0 */
    USB_WBVAL(USB_VID), /* idVendor */
    USB_WBVAL(USB_PID), /* idProduct */
    USB_WBVAL(USB_REV), /* bcdDevice */
    0x01,   /* iManufacturer USB_DEVICE_STRING_RESERVED*/
    0x02,   /* iProduct USB_DEVICE_STRING_RESERVED*/
    0x03,   /* iSerialNumber USB_DEVICE_STRING_RESERVED*/
    0x01    /* bNumConfigurations: one possible configuration*/
};
// 配置描述符
const uint8_t MyCfgDescr[] = {
    0x09,   /* bLength */
    0x02,   /* bDescriptorType USB_CONFIGURATION_DESCRIPTOR_TYPE*/
    USB_WBVAL(9+9+9+7+9+9+7+9+7+9+9+9+9+7+7),   /* wTotalLength */
//    USB_WBVAL(9+9+9+7+9+9+7+9+9+7+7+9+9+9+9+7+7),   /* wTotalLength */
    USB_INTERFACE_MAX_NUM,   /* bNumInterfaces */
    0x01,   /* bConfigurationValue */
    0x00,   /* iConfiguration */
    0xA0,
    0x23,    /* bMaxPower */
    //配置描述符

//   -----Interface 描述符 Mouse --------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x00,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    //接口描述符

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeMouse),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x81,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP1 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP2SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */
    //端点2描述符

//   -----Interface 描述符 Other --------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x01,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x01,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    //接口描述符

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeOther),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x82,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP2 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP2SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */

//   -----Interface 描述符 Other --------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x02,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x02,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x00,   /* bInterfaceSubClass */
    0x00,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    //接口描述符

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeManufacturer),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x83,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP3 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP3SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x03,   /* bEndpointAddress *//* USB_ENDPOINT_OUT USB_EP3 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP3SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */
    //端点5描述符

////   -----Interface 描述符 Other --------------
//    0x09,   /* bLength */
//    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
//    0x03,   /* bInterfaceNumber */
//    0x00,   /* bAlternateSetting */
//    0x00,   /* bNumEndpoints */
//    0x03,   /* bInterfaceClass */
//    0x00,   /* bInterfaceSubClass */
//    0x00,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
//    0x00,   /* iInterface */
//
//    //接口描述符
//    0x09,   /* bLength */
//    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
//    USB_WBVAL(0x0100),/* bcdHID */
//    0x00,   /* bCountryCode */
//    0x00,   /* bNumDescriptors */
//    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
//    USB_WBVAL(0),/* wDescriptorLength */
    //HID类描述符

//   -----Interface 描述符 Other --------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x03,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x02,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x00,   /* bInterfaceSubClass */
    0x00,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    //接口描述符

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeIAP),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x86,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP6 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP6SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x06,   /* bEndpointAddress *//* USB_ENDPOINT_OUT USB_EP6 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP6SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */
    //端点6描述符

};

__attribute__((aligned(4)))  uint8_t EP0_Databuf[8]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4)))  uint8_t EP1_Databuf[64 + 64];    //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4)))  uint8_t EP2_Databuf[64 + 64];    //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4)))  uint8_t EP3_Databuf[64 + 64];    //ep3_out(64)+ep3_in(64)
__attribute__((aligned(4)))  uint8_t EP6_Databuf[64 + 64];    //ep5_out(64)+ep5_in(64)

uint8_t        DevConfig= 0;
uint8_t        SetupReqCode;
uint16_t       SetupReqLen;
const uint8_t *pDescr;
uint8_t        Report_Value[USB_INTERFACE_MAX_INDEX+1] = {0x00};
uint8_t        Idle_Value[USB_INTERFACE_MAX_INDEX+1] = {0x00};
volatile uint8_t        USB_SleepStatus = HOST_WAKEUP_ENABLE; /* USB睡眠状态 */
volatile uint8_t    USB_READY_FLAG = 1;
USB_receive_cb_t    USB_receive_cb;
uint8_t         led_val = 0;
uint8_t         USB_receive_buf[64+2];
volatile uint8_t intflag = 0;   //intflag用于存放标志寄存器值
uint8_t         usb_enum_flag = 0;
uint8_t         usb_enum_success_flag=0;

void USB_receive_process(uint8_t *pData,uint8_t len)
{
    if(pData[0] == USB_DATA_IAP)
    {
        OTA_USB_IAPWriteData(&pData[2], len-2);
    }
    else if(pData[0] == USB_DATA_LED)
    {
        peripheral_pilot_led_receive(pData[2]);
    }
    else if(pData[0] == USB_DATA_MANUFACTURER)
    {
        PRINT("MANUFACTURER %x\n",pData[2]);
    }
}

/*********************************************************************
 * @fn      USB_Init
 *
 * @brief   USB_Init
 *
 * @return  none
 */
void USB_Init( void )
{
    PFIC_DisableIRQ(USB_IRQn);
    pEP0_RAM_Addr = EP0_Databuf;
    pEP1_RAM_Addr = EP1_Databuf;
    pEP2_RAM_Addr = EP2_Databuf;
    pEP3_RAM_Addr = EP3_Databuf;
    pEP6_RAM_Addr = EP6_Databuf;
    USB_DeviceInit();
    USB_receive_cb_register(USB_receive_process);
    PFIC_EnableIRQ( USB_IRQn );
}

/*********************************************************************
 * @fn      USB_Uinit
 *
 * @brief   USB_Uinit
 *
 * @return  none
 */
void USB_Uinit( void )
{
    R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_IE | RB_PIN_USB_DP_PU);
    R8_UDEV_CTRL = 0;
    R8_USB_CTRL = 0;
    R8_USB_INT_EN = 0;
    GPIOB_ModeCfg( GPIO_Pin_10 | GPIO_Pin_11, GPIO_ModeIN_PD);
    PFIC_DisableIRQ(USB_IRQn);
}

/*********************************************************************
 * @fn      USB_cfg_vid_pid
 *
 * @brief   USB_cfg_vid_pid
 *
 * @return  none
 */
void USB_cfg_vid_pid( uint16_t VID, uint16_t PID )
{
    MyDevDescr[8] = VID& 0xFF;
    MyDevDescr[9] = (VID >> 8) & 0xFF;
    MyDevDescr[10] = PID& 0xFF;
    MyDevDescr[11] = (PID >> 8) & 0xFF;
}

/*********************************************************************
 * @fn      USB_cfg_manu_info
 *
 * @brief   USB_cfg_manu_info
 *
 * @return  none
 */
void USB_cfg_manu_info( uint8_t *pData, uint8_t len )
{
    uint8_t i;
    MyManuInfo[0] = len*2+2;
    MyManuInfo[1] = 0x03;
    for(i=0; i<len; i++)
    {
        MyManuInfo[2+i*2] = pData[i];
        MyManuInfo[3+i*2] = 0x00;
    }
}

/*********************************************************************
 * @fn      USB_cfg_prod_info
 *
 * @brief   USB_cfg_prod_info
 *
 * @return  none
 */
void USB_cfg_prod_info( uint8_t *pData, uint8_t len )
{
    uint8_t i;
    MyProdInfo[0] = len*2+2;
    MyProdInfo[1] = 0x03;
    for(i=0; i<len; i++)
    {
        MyProdInfo[2+i*2] = pData[i];
        MyProdInfo[3+i*2] = 0x00;
    }
}

/*********************************************************************
 * @fn      USB_cfg_serial_num
 *
 * @brief   USB_cfg_serial_num
 *
 * @return  none
 */
void USB_cfg_serial_num( uint8_t *pData, uint8_t len )
{
    uint8_t i;
    MySerialNum[0] = len*2+2;
    MySerialNum[1] = 0x03;
    for(i=0; i<len; i++)
    {
        MySerialNum[2+i*2] = pData[i];
        MySerialNum[3+i*2] = 0x00;
    }
}
/*********************************************************************
 * @fn      USB_Wake_up
 *
 * @brief   USB_Wake_up
 *
 * @return  none
 */
void USB_Wake_up( void )
{
    R16_PIN_ANALOG_IE &= ~RB_PIN_USB_DP_PU;         // USB上拉电阻
    R8_UDEV_CTRL |= RB_UD_LOW_SPEED;
    mDelaymS(8);
    R8_UDEV_CTRL &= ~RB_UD_LOW_SPEED;
    R16_PIN_ANALOG_IE |= RB_PIN_USB_DP_PU;         // USB上拉电阻
}

/*********************************************************************
 * @fn      USB_receive_cb_register
 *
 * @brief   USB_receive_cb_register
 *
 * @return  none
 */
void USB_receive_cb_register(USB_receive_cb_t cback)
{
    USB_receive_cb = cback;
}

/*********************************************************************
 * @fn      USB_mouse_report
 *
 * @brief   上报mouse数据
 *
 * @return  0：成功
 *          1：出错
 */
uint8_t USB_mouse_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    tmos_memcpy(pEP1_IN_DataBuf, pData, len);
    DevEP1_IN_Deal(len);
    return 0;
}

/*********************************************************************
 * @fn      USB_class_keyboard_report
 *
 * @brief   上报class_keyboard数据
 *
 * @return  0：成功
 *          1：出错
 */
uint8_t USB_class_keyboard_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_KEYBOARD;
    tmos_memcpy(&pEP2_IN_DataBuf[1], pData, len);
    DevEP2_IN_Deal(len+1);
    return 0;
}

/*********************************************************************
 * @fn      USB_all_keyboard_report
 *
 * @brief   上报all_keyboard数据
 *
 * @return  0：成功
 *          1：出错
 */
uint8_t USB_all_keyboard_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_ALL_KEYBOARD;
    tmos_memcpy(&pEP2_IN_DataBuf[1], pData, len);
    DevEP2_IN_Deal(len+1);
    return 0;
}
/*********************************************************************
 * @fn      USB_manufacturer_report
 *
 * @brief   上报manufacturer数据，用户注意做好上层协议
 *
 * @return  0：成功
 *          1：出错
 */
uint8_t USB_manufacturer_report(uint8_t *pData,uint8_t len)
{
    USB_READY_FLAG = 0;
    tmos_memcpy(pEP3_IN_DataBuf, pData, len);
    DevEP3_IN_Deal(len);
    return 0;
}

/*********************************************************************
 * @fn      USB_consumer_report
 *
 * @brief   上报consumer数据
 *
 * @return  0：成功
 *          1：出错
 */
uint8_t USB_consumer_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_CONSUMER;
    tmos_memcpy(&pEP2_IN_DataBuf[1], pData, len);
    DevEP2_IN_Deal(len+1);
    return 0;
}

/*********************************************************************
 * @fn      USB_sys_ctl_report
 *
 * @brief   上报sys_ctl数据
 *
 * @return  0：成功
 *          1：出错
 */
uint8_t USB_sys_ctl_report(uint8_t data)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_SYS_CTL;
    pEP2_IN_DataBuf[1] = data;
    DevEP2_IN_Deal(2);
    return 0;
}

/*********************************************************************
 * @fn      USB_IAP_report
 *
 * @brief   上报IAP数据
 *
 * @return  0：成功
 *          1：出错
 */
uint8_t USB_IAP_report(uint8_t *pData,uint8_t len)
{
    USB_READY_FLAG = 0;
    tmos_memcpy(pEP6_IN_DataBuf, pData, len);
    DevEP6_IN_Deal(0x40);
    return 0;
}

/*********************************************************************
 * @fn      USB_IRQ_trans_process
 *
 * @brief   USB中断函数
 *
 * @return  none
 */
void USB_IRQ_trans_process( void )
{
    uint8_t len, chtype;        //len用于拷贝函数，chtype用于存放数据传输方向、命令的类型、接收的对象等信息
    uint8_t  errflag = 0;   //errflag用于标记是否支持主机的指令

    if( intflag & RB_UIF_TRANSFER )   //判断_INT_FG中的USB传输完成中断标志位。若有传输完成中断了，进if语句
    {
        if( (R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN ) // 非空闲   //判断中断状态寄存器中的5:4位，查看令牌的PID标识。若这两位不是11（表示空闲），进if语句
        {
            switch( R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP) )
            //取得令牌的PID标识和设备模式下的3:0位的端点号。主机模式下3:0位是应答PID标识位
            // 分析操作令牌和端点号
            {//端点0用于控制传输。以下的端点0的IN和OUT令牌相应程序，对应控制传输的数据阶段和状态阶段。
                case UIS_TOKEN_IN:      //令牌包的PID为IN，5:4位为10。3:0位的端点号为0。IN令牌：设备给主机发数据。_UIS_：USB中断状态
                {                       //端点0为双向端点，用作控制传输。 “|0”运算省略了
                    switch( SetupReqCode )
                    //这个值会在收到SETUP包时赋值。在后面会有SETUP包处理程序，对应控制传输的设置阶段。
                    {
                        case USB_GET_DESCRIPTOR:    //USB标准命令，主机从USB设备获取描述
                            len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen; // 本次包传输长度。最长为64字节，超过64字节的分多次处理，前几次要满包。
                            tmos_memcpy( pEP0_DataBuf, pDescr, len ); //tmos_memcpy:内存拷贝函数，从(二号位)地址拷贝(三号位)字符串长度到(一号位)地址中
                            //DMA直接与内存相连，会检测到内存的改写，而后不用单片机控制就可以将内存中的数据发送出去。如果只是两个数组互相赋值，不涉及与DMA匹配的物理内存，就无法触发DMA。
                            SetupReqLen -= len;     //记录剩下的需要发送的数据长度
                            pDescr += len;          //更新接下来需要发送的数据的起始地址,拷贝函数用
                            R8_UEP0_T_LEN = len;    //端点0发送长度寄存器中写入本次包传输长度
                            R8_UEP0_CTRL ^= RB_UEP_T_TOG;   // 同步切换。IN方向（对于单片机就是T方向）的PID中的DATA0和DATA1切换
                            break;                  //赋值完端点控制寄存器的握手包响应（ACK、NAK、STALL），由硬件打包成符合规范的包，DMA自动发送
                        case USB_SET_ADDRESS:       //USB标准命令，主机为设备设置一个唯一地址，范围0～127，0为默认地址
                            R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                            //7位地址+最高位的用户自定义地址（默认为1），或上“包传输长度”（这里的“包传输长度”在后面赋值成了地址位）
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            //R响应OUT事务ACK，T响应IN事务NAK。这个CASE分支里是IN方向，当DMA相应内存中，单片机没有数据更新时，回NAK握手包。
                            break;                                                  //一般程序里的OUT事务，设备会回包给主机，不响应NAK。

                        case USB_SET_FEATURE:       //USB标准命令，主机要求启动一个在设备、接口或端点上的特征
                            break;

                        default:
                            R8_UEP0_T_LEN = 0;      //状态阶段完成中断或者是强制上传0长度数据包结束控制传输（数据字段长度为0的数据包，包里SYNC、PID、EOP字段都有）
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            //R响应OUT事务ACK，T响应IN事务NAK。这个CASE分支里是OUT方向，当DMA相应内存中更新了数据且单片机验收正常时，回ACK握手包。
                            USB_READY_FLAG = 1;
                            if(usb_enum_flag)
                            {
                                usb_enum_success_flag = 1;
                            }
#if CONFIG_USB_DEBUG
                            PRINT( "Ready_STATUS = %d\n", USB_READY_FLAG );
#endif
                            break;
                    }
                }
                    break;

                case UIS_TOKEN_OUT:     //令牌包的PID为OUT，5:4位为00。3:0位的端点号为0。OUT令牌：主机给设备发数据。
                {                       //端点0为双向端点，用作控制传输。 “|0”运算省略了
                    len = R8_USB_RX_LEN;    //读取当前USB接收长度寄存器中存储的接收的数据字节数 //接收长度寄存器为各个端点共用，发送长度寄存器各有各的
                    if((pEP0_DataBuf[0]==REPORT_ID_KEYBOARD)&&USB_receive_cb&&(len==2))
                    {
                        USB_receive_buf[0] = USB_DATA_LED;
                        USB_receive_buf[1] = 1;
                        USB_receive_buf[2] = pEP0_DataBuf[1];
                        led_val = pEP0_DataBuf[1];
                        USB_receive_cb(USB_receive_buf, USB_receive_buf[1]+2);
#if CONFIG_USB_DEBUG
                        PRINT( "OUT 0  %x len %d\n", pEP0_DataBuf[1],len );
#endif
                    }
                }
                    break;

                case UIS_TOKEN_OUT | 3:
                {
                    R8_UEP3_CTRL ^= RB_UEP_R_TOG;       //IN事务的DATA切换一下。设定将要发送的包的PID。
                    len = R8_USB_RX_LEN;    //读取当前USB接收长度寄存器中存储的接收的数据字节数 //接收长度寄存器为各个端点共用，发送长度寄存器各有各的
                    if(USB_receive_cb)
                    {
                        USB_receive_buf[0] = USB_DATA_MANUFACTURER;
                        USB_receive_buf[1] = len;
                        tmos_memcpy(&USB_receive_buf[2], &pEP3_OUT_DataBuf[0], len);
                        USB_receive_cb(USB_receive_buf, len+2);
#if CONFIG_USB_DEBUG
                        PRINT( "OUT 3 %x len %d\n", pEP3_OUT_DataBuf[0],len );
#endif
                    }
                }
                    break;

                case UIS_TOKEN_OUT | 6:
                {
                    R8_UEP6_CTRL ^= RB_UEP_R_TOG;       //IN事务的DATA切换一下。设定将要发送的包的PID。
                    len = R8_USB_RX_LEN;    //读取当前USB接收长度寄存器中存储的接收的数据字节数 //接收长度寄存器为各个端点共用，发送长度寄存器各有各的
                    if(USB_receive_cb)
                    {
                        USB_receive_buf[0] = USB_DATA_IAP;
                        USB_receive_buf[1] = len;
                        tmos_memcpy(&USB_receive_buf[2], &pEP6_OUT_DataBuf[0], len);
                        USB_receive_cb(USB_receive_buf, len+2);
#if CONFIG_USB_DEBUG
                        PRINT( "OUT 6 %x len %d\n", pEP6_OUT_DataBuf[0],len );
#endif
                    }
                }
                    break;

                case UIS_TOKEN_IN | 1: //令牌包的PID为IN，端点号为1
                    R8_UEP1_CTRL ^= RB_UEP_T_TOG;       //IN事务的DATA切换一下。设定将要发送的包的PID。
                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; //当DMA中没有由单片机更新数据时，将T响应IN事务置为NAK。更新了就发出数据。
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP1 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 2: //令牌包的PID为IN，端点号为2
                    R8_UEP2_CTRL ^= RB_UEP_T_TOG;       //IN事务的DATA切换一下。设定将要发送的包的PID。
                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; //当DMA中没有由单片机更新数据时，将T响应IN事务置为NAK。更新了就发出数据。
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP2 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 3: //令牌包的PID为IN，端点号为3
                    R8_UEP3_CTRL ^= RB_UEP_T_TOG;       //IN事务的DATA切换一下。设定将要发送的包的PID。
                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; //当DMA中没有由单片机更新数据时，将T响应IN事务置为NAK。更新了就发出数据。
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP3 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 5: //令牌包的PID为IN，端点号为5
                    R8_UEP5_CTRL ^= RB_UEP_T_TOG;       //IN事务的DATA切换一下。设定将要发送的包的PID。
                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; //当DMA中没有由单片机更新数据时，将T响应IN事务置为NAK。更新了就发出数据。
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP5 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 6: //令牌包的PID为IN，端点号为3
                    R8_UEP6_CTRL ^= RB_UEP_T_TOG;       //IN事务的DATA切换一下。设定将要发送的包的PID。
                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; //当DMA中没有由单片机更新数据时，将T响应IN事务置为NAK。更新了就发出数据。
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP6 = %d\n", USB_READY_FLAG );
#endif
                    break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;    //写1清零中断标志
        }

        if( R8_USB_INT_ST & RB_UIS_SETUP_ACT ) // Setup包处理
        {
//            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            //R响应OUT事务期待为DATA1（DMA收到的数据包的PID要为DATA1，否则算数据错误要重传）和ACK（DMA相应内存中收到了数据，单片机验收正常）
            //T响应IN事务设定为DATA1（单片机有数据送入DMA相应内存，以DATA1发送出去）和NAK（单片机没有准备好数据）。
            SetupReqLen = pSetupReqPak->wLength;    //数据阶段的字节数      //pSetupReqPak：将端点0的RAM地址强制转换成一个存放结构体的地址，结构体成员依次紧凑排列
            SetupReqCode = pSetupReqPak->bRequest;  //命令的序号
            chtype = pSetupReqPak->bRequestType;    //包含数据传输方向、命令的类型、接收的对象等信息

            len = 0;
            errflag = 0;
            if( (pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD ) //判断命令的类型，如果不是标准请求，进if语句
            {
                /* 非标准请求 */
                /* 其它请求,如类请求，产商请求等 */
                if( pSetupReqPak->bRequestType & 0x40 )   //取得命令中的某一位，判断是否为0，不为零进if语句
                {
                    /* 厂商请求 */
                }
                else if( pSetupReqPak->bRequestType & 0x20 )  //取得命令中的某一位，判断是否为0，不为零进if语句
                {   //判断为HID类请求
                    switch( SetupReqCode )
                    //判断命令的序号
                    {
                        case DEF_USB_SET_IDLE: /* 0x0A: SET_IDLE */         //主机想设置HID设备特定输入报表的空闲时间间隔
                            Idle_Value[pSetupReqPak->wIndex] = (uint8_t)(pSetupReqPak->wValue>>8);
                            break; //这个一定要有

                        case DEF_USB_SET_REPORT: /* 0x09: SET_REPORT */     //主机想设置HID设备的报表描述符
                            break;

                        case DEF_USB_SET_PROTOCOL: /* 0x0B: SET_PROTOCOL */ //主机想设置HID设备当前所使用的协议
                            Report_Value[pSetupReqPak->wIndex] = (uint8_t)(pSetupReqPak->wValue);
                            break;

                        case DEF_USB_GET_IDLE: /* 0x02: GET_IDLE */         //主机想读取HID设备特定输入报表的当前的空闲比率
                            EP0_Databuf[0] = Idle_Value[pSetupReqPak->wIndex];
                            len = 1;
                            break;

                        case DEF_USB_GET_PROTOCOL: /* 0x03: GET_PROTOCOL */     //主机想获得HID设备当前所使用的协议
                            EP0_Databuf[0] = Report_Value[pSetupReqPak->wIndex];
                            len = 1;
                            break;

                        default:
                            errflag = 0xFF;
                    }
                }
            }
            else    //判断为标准请求
            {
                switch( SetupReqCode )
                //判断命令的序号
                {
                    case USB_GET_DESCRIPTOR:    //主机想获得标准描述符
                    {
                        switch( ((pSetupReqPak->wValue) >> 8) )
                        //右移8位，看原来的高8位是否为0，为1表示方向为IN方向，则进s-case语句
                        {
                            case USB_DESCR_TYP_DEVICE:  //不同的值代表不同的命令。主机想获得设备描述符
                            {
                                tmos_stop_task(tran_taskID, SBP_ENTER_SLEEP_EVT);
                                pDescr = MyDevDescr;    //将设备描述符字符串放在pDescr地址中，“获得标准描述符”这个case末尾会用拷贝函数发送
                                len = MyDevDescr[0];    //协议规定设备描述符的首字节存放字节数长度。拷贝函数会用到len参数
                            }
                                break;

                            case USB_DESCR_TYP_CONFIG:  //主机想获得配置描述符
                            {
                                pDescr = MyCfgDescr;    //将配置描述符字符串放在pDescr地址中，之后会发送
                                len = MyCfgDescr[2];    //协议规定配置描述符的第三个字节存放配置信息的总长
                            }
                                break;

                            case USB_DESCR_TYP_HID:     //主机想获得人机接口类描述符。此处结构体中的wIndex与配置描述符不同，意为接口号。
                                switch( (pSetupReqPak->wIndex) & 0xff )
                                //取低八位，高八位抹去
                                {
                                    /* 选择接口 */
                                    case 0:
                                        pDescr = (uint8_t *) (&MyCfgDescr[18]);  //接口1的类描述符存放位置，待发送
                                        len = 9;
                                        break;

                                    /* 选择接口 */
                                    case 1:
                                        pDescr = (uint8_t *) (&MyCfgDescr[43]);  //接口1的类描述符存放位置，待发送
                                        len = 9;
                                        break;

                                    /* 选择接口 */
                                    case 2:
                                        pDescr = (uint8_t *) (&MyCfgDescr[68]);  //接口1的类描述符存放位置，待发送
                                        len = 9;
                                        break;

                                    /* 选择接口 */
                                    case 3:
                                        pDescr = (uint8_t *) (&MyCfgDescr[68+25+7]);  //接口1的类描述符存放位置，待发送
                                        len = 9;
                                        break;

//                                    /* 选择接口 */
//                                    case 4:
//                                        pDescr = (uint8_t *) (&MyCfgDescr[68+25+9+9]);  //接口1的类描述符存放位置，待发送
//                                        len = 9;
//                                        break;

                                    default:
                                        /* 不支持的字符串描述符 */
                                        errflag = 0xff;
                                        break;
                                }
                                break;

                            case USB_DESCR_TYP_REPORT:  //主机想获得设备报表描述符
                            {
                                if( ((pSetupReqPak->wIndex) & 0xff) == 0 ) //接口0报表描述符
                                {
                                    pDescr = HID_ReportDescriptorMouse; //数据准备上传
                                    len = HID_ReportDescSizeMouse;
                                }
                                else if( ((pSetupReqPak->wIndex) & 0xff) == 1 ) //接口1报表描述符
                                {
                                    pDescr = HID_ReportDescriptorOther; //数据准备上传
                                    len = HID_ReportDescSizeOther;
                                }
                                else if( ((pSetupReqPak->wIndex) & 0xff) == 2 ) //接口2报表描述符
                                {
                                    pDescr = HID_ReportDescriptorManufacturer; //数据准备上传
                                    len = HID_ReportDescSizeManufacturer;
                                }
                                else if( ((pSetupReqPak->wIndex) & 0xff) == 3 ) //接口3报表描述符
                                {
                                    usb_enum_flag = 1;
                                    pDescr = HID_ReportDescriptorIAP; //数据准备上传
                                    len = HID_ReportDescSizeIAP;
                                }
                                else
                                    len = 0xff; //本程序只有4个接口，这句话正常不可能执行
                            }
                                break;

                            case USB_DESCR_TYP_STRING:  //主机想获得设备字符串描述符
                            {
                                switch( (pSetupReqPak->wValue) & 0xff )
                                //根据wValue的值传递字符串信息
                                {
                                    case 0:
                                        pDescr = LangID_StrDescr;
                                        len = LangID_StrDescr[0];
                                        break;
                                    case 1:
                                        pDescr = MyManuInfo;
                                        len = MyManuInfo[0];
                                        break;
                                    case 2:
                                        pDescr = MyProdInfo;
                                        len = MyProdInfo[0];
                                        break;
                                    case 3:
                                        pDescr = MySerialNum;
                                        len = MySerialNum[0];
                                        break;
                                    default:
                                        errflag = 0xFF; // 不支持的字符串描述符
                                        break;
                                }
                            }
                                break;

                            default:
                                errflag = 0xff;
                                break;
                        }
                        if( SetupReqLen > len )
                            SetupReqLen = len;      //实际需上传总长度
                        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;   //最大长度为64字节
                        tmos_memcpy( pEP0_DataBuf, pDescr, len );  //拷贝函数
                        pDescr += len;
                    }
                        break;

                    case USB_SET_ADDRESS:       //主机想设置设备地址
                        SetupReqLen = (pSetupReqPak->wValue) & 0xff;    //将主机分发的位设备地址暂存在SetupReqLen中
                        break;                                          //控制阶段会赋值给设备地址参数

                    case USB_GET_CONFIGURATION: //主机想获得设备当前配置
                        pEP0_DataBuf[0] = DevConfig;    //将设备配置放进RAM
                        if( SetupReqLen > 1 )
                            SetupReqLen = 1;    //将数据阶段的字节数置1。因为DevConfig只有一个字节
                        break;

                    case USB_SET_CONFIGURATION: //主机想设置设备当前配置
                        DevConfig = (pSetupReqPak->wValue) & 0xff;  //取低八位，高八位抹去
                        break;

                    case USB_CLEAR_FEATURE:     //关闭USB设备的特征/功能。可以是设备或是端点层面上的。
                    {
                        if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP ) //判断是不是端点特征（清除端点停止工作的状态）
                        {
                            switch( (pSetupReqPak->wIndex) & 0xff )
                            //取低八位，高八位抹去。判断索引
                            {//16位的最高位判断数据传输方向，0为OUT，1为IN。低位为端点号。
                                case 0x81:      //清零_TOG和_T_RES这三位，并将后者写成_NAK，响应IN事务NAK表示无数据返回
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x01:      //清零_TOG和_R_RES这三位，并将后者写成_ACK，响应OUT事务ACK表示接收正常
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x82:      //清零_TOG和_T_RES这三位，并将后者写成_NAK，响应IN事务NAK表示无数据返回
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x02:      //清零_TOG和_R_RES这三位，并将后者写成_ACK，响应OUT事务ACK表示接收正常
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x83:      //清零_TOG和_T_RES这三位，并将后者写成_NAK，响应IN事务NAK表示无数据返回
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x03:      //清零_TOG和_R_RES这三位，并将后者写成_ACK，响应OUT事务ACK表示接收正常
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x85:      //清零_TOG和_T_RES这三位，并将后者写成_NAK，响应IN事务NAK表示无数据返回
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x05:      //清零_TOG和_R_RES这三位，并将后者写成_ACK，响应OUT事务ACK表示接收正常
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x86:      //清零_TOG和_T_RES这三位，并将后者写成_NAK，响应IN事务NAK表示无数据返回
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x06:      //清零_TOG和_R_RES这三位，并将后者写成_ACK，响应OUT事务ACK表示接收正常
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                default:
                                    errflag = 0xFF; // 不支持的端点
                                    break;
                            }
                        }
                        else if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE ) //判断是不是设备特征（用于设备唤醒）
                        {
#if CONFIG_USB_DEBUG
                            PRINT( "清睡眠\n" );
#endif
                            if( pSetupReqPak->wValue == 1 )   //唤醒标志位为1
                            {
                                USB_SleepStatus &= ~HOST_SET_FEATURE;   //最低位清零
                                USB_SleepStatus |= HOST_WAKEUP_ENABLE;
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                    }
                        break;

                    case USB_SET_FEATURE:       //开启USB设备的特征/功能。可以是设备或是端点层面上的。
                        if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP ) //判断是不是端点特征（使端点停止工作）
                        {
                            /* 端点 */
                            switch( pSetupReqPak->wIndex )
                            //判断索引
                            {//16位的最高位判断数据传输方向，0为OUT，1为IN。低位为端点号。
                                case 0x81:      //清零_TOG和_T_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x01:      //清零_TOG和_R_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x82:      //清零_TOG和_T_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x02:      //清零_TOG和_R_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x83:      //清零_TOG和_T_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x03:      //清零_TOG和_R_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x85:      //清零_TOG和_T_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x05:      //清零_TOG和_R_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x86:      //清零_TOG和_T_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x06:      //清零_TOG和_R_RES三位，并将后者写成_STALL，根据主机指令停止端点的工作
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                default:
                                    /* 不支持的端点 */
                                    errflag = 0xFF; // 不支持的端点
                                    break;
                            }
                        }
                        else if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE ) //判断是不是设备特征（使设备休眠）
                        {
#if CONFIG_USB_DEBUG
                            PRINT( "设置睡眠\n" );
#endif
                            if( pSetupReqPak->wValue == 1 )
                            {
                                USB_SleepStatus |= HOST_SET_FEATURE;    //设置睡眠
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                        break;

                    case USB_GET_INTERFACE:     //主机想获得接口当前工作的选择设置值
                        pEP0_DataBuf[0] = 0x00;
                        if( SetupReqLen > 1 )
                            SetupReqLen = 1;    //将数据阶段的字节数置1。因为待传数据只有一个字节
                        break;

                    case USB_SET_INTERFACE:     //主机想激活设备的某个接口
                        break;

                    case USB_GET_STATUS:        //主机想获得设备、接口或是端点的状态
                        if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP ) //判断是否为端点状态
                        {
                            /* 端点 */
                            pEP0_DataBuf[0] = 0x00;
                            switch( pSetupReqPak->wIndex )
                            {       //16位的最高位判断数据传输方向，0为OUT，1为IN。低位为端点号。
                                case 0x81:      //判断_TOG和_T_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP1_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; //返回D0为1，表示端点被停止工作了。该位由SET_FEATURE和CLEAR_FEATURE命令配置。
                                    }
                                    break;

                                case 0x01:      //判断_TOG和_R_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP1_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x82:      //判断_TOG和_T_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP2_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; //返回D0为1，表示端点被停止工作了。该位由SET_FEATURE和CLEAR_FEATURE命令配置。
                                    }
                                    break;

                                case 0x02:      //判断_TOG和_R_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP2_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x83:      //判断_TOG和_T_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP3_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; //返回D0为1，表示端点被停止工作了。该位由SET_FEATURE和CLEAR_FEATURE命令配置。
                                    }
                                    break;

                                case 0x03:      //判断_TOG和_R_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP3_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x85:      //判断_TOG和_T_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP5_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; //返回D0为1，表示端点被停止工作了。该位由SET_FEATURE和CLEAR_FEATURE命令配置。
                                    }
                                    break;

                                case 0x05:      //判断_TOG和_R_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP5_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x86:      //判断_TOG和_T_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP6_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; //返回D0为1，表示端点被停止工作了。该位由SET_FEATURE和CLEAR_FEATURE命令配置。
                                    }
                                    break;

                                case 0x06:      //判断_TOG和_R_RES三位，若处于STALL状态，进if语句
                                    if( (R8_UEP6_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                            }
                        }
                        else if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE ) //判断是否为设备状态
                        {
                            pEP0_DataBuf[0] = 0x00;
                            if( USB_SleepStatus&HOST_SET_FEATURE )     //如果设备处于睡眠状态
                            {
                                pEP0_DataBuf[0] = 0x02;     //最低位D0为0，表示设备由总线供电，为1表示设备自供电。 D1位为1表示支持远程唤醒，为0表示不支持。
                            }
                            else
                            {
                                pEP0_DataBuf[0] = 0x00;
                            }
                        }
                        pEP0_DataBuf[1] = 0;    //返回状态信息的格式为16位数，高八位保留为0
                        if( SetupReqLen >= 2 )
                        {
                            SetupReqLen = 2;    //将数据阶段的字节数置2。因为待传数据只有2个字节
                        }
                        break;

                    default:
                        errflag = 0xff;
                        break;
                }
            }
            if( errflag == 0xff ) // 错误或不支持
            {
                //                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
                USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                PRINT( "Ready_Stall = %d\n", USB_READY_FLAG );
#endif
            }
            else
            {
                if( chtype & 0x80 )   // 上传。最高位为1，数据传输方向为设备向主机传输。
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                    len = 0;        // 下传。最高位为0，数据传输方向为主机向设备传输。
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;     // 默认数据包是DATA1
            }

            R8_USB_INT_FG = RB_UIF_TRANSFER;    //写1清中断标识
        }
    }

    else if( intflag & RB_UIF_BUS_RST )   //判断_INT_FG中的总线复位标志位，为1触发
    {
        USB_READY_FLAG = 1;
        tmos_stop_task(tran_taskID, SBP_ENTER_SLEEP_EVT);
        R8_USB_DEV_AD = 0;      //设备地址写成0，待主机重新分配给设备一个新地址
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;   //把端点0的控制寄存器，写成：接收响应响应ACK表示正常收到，发送响应NAK表示没有数据要返回
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP5_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP6_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_USB_INT_FG = RB_UIF_BUS_RST; //写1清中断标识
    }
    else if( intflag & RB_UIF_SUSPEND )   //判断_INT_FG中的总线挂起或唤醒事件中断标志位。挂起和唤醒都会触发此中断
    {
        if( R8_USB_MIS_ST & RB_UMS_SUSPEND )  //取得杂项状态寄存器中的挂起状态位，为1表示USB总线处于挂起态，为0表示总线处于非挂起态
        {
            USB_READY_FLAG = 0;
            USB_SleepStatus |= HOST_SET_SUSPEND;
            if(USB_SleepStatus & HOST_SET_FEATURE)
            {
                tmos_set_event(tran_taskID, SBP_ENTER_SLEEP_EVT);
            }
#if CONFIG_USB_DEBUG
            PRINT( "Ready_Sleep = %x\n", USB_SleepStatus );
#endif
        } // 挂起     //当设备处于空闲状态超过3ms，主机会要求设备挂起（类似于电脑休眠）
        else    //挂起或唤醒中断被触发，又没有被判断为挂起
        {
            USB_READY_FLAG = 1;
            USB_SleepStatus &= ~HOST_SET_SUSPEND;
            USB_SleepStatus |= HOST_WAKEUP_ENABLE;
            tmos_stop_task(tran_taskID, SBP_ENTER_SLEEP_EVT);
#if CONFIG_USB_DEBUG
            PRINT( "Ready_WeakUp = %x\n", USB_SleepStatus );
#endif
        } // 唤醒
        R8_USB_INT_FG = RB_UIF_SUSPEND; //写1清中断标志
    }
    else
    {
        R8_USB_INT_FG = intflag;    //_INT_FG中没有中断标识，再把原值写回原来的寄存器
    }
}

/*********************************************************************
 * @fn      USB_IRQHandler
 *
 * @brief   USB中断函数
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void USB_IRQHandler( void ) /* USB中断服务程序,使用寄存器组1 */
{
    intflag = R8_USB_INT_FG;        //取得中断标识寄存器的值

    if( intflag & RB_UIF_TRANSFER )   //判断_INT_FG中的USB传输完成中断标志位。若有传输完成中断了，进if语句
    {
        if( R8_USB_INT_ST & RB_UIS_SETUP_ACT ) // Setup包处理
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        }
    }
    USB_IRQ_trans_process();

}

/******************************** endfile @ usb ******************************/
