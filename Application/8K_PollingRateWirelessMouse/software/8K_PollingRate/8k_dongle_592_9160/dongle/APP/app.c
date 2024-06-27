/********************************** (C) COPYRIGHT *******************************
 * File Name          : app.c
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
#include "ch9160.h"
#include <rf.h>
#include "HAL.h"
#include "nvs_flash.h"
#include "app.h"
#include "ota.h"
#include <stdarg.h>
#include <stdio.h>

uint8_t app_taskID;

// 支持的最大接口数量
#define USB_INTERFACE_MAX_NUM       4
// 重传间隔，单位为1个RTC时钟
#define RETRAN_INTERVAL         32

const uint8_t LangID_StrDescr[ 4 ] =
{
    0x04,
    0x03,                                                                       /* bDescriptorType */
    0x09,
    0x04
};

// 厂家信息
uint8_t MyManuInfo[] = {0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0};

// 产品信息
uint8_t MyProdInfo[] = {0x0D+11, 0x03,
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
uint8_t MySerialNum[] = {0x0D+11, 0x03,
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
//   -----报告描述符Keyboard--------------
const uint8_t HID_ReportDescriptorKeyboard[]=
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
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
    0x95, 0x03,        //   Report Count (3)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x03,        //   Usage Maximum (Scroll Lock)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x08,        //   Report Size (8)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0xFF,        //   Usage Maximum (0xFF)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0xFF,        //   Usage Page (Reserved 0xFF)
    0x09, 0x03,        //   Usage (0x03)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection

    // 72 bytes

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
        0xC0,                       //134   MAIN_COLLECTION_END
};


//   -----报告描述符Other--------------
const uint8_t HID_ReportDescriptorOther[]=
{
    0x05,0x0C,                  //0     GLOBAL_USAGE_PAGE(Consumer)
    0x09,0x01,                  //2     LOCAL_USAGE(    Consumer Control    )
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
        0x85,REPORT_ID_CONSUMER,                  //6     GLOBAL_REPORT_ID(3)
        0x19,0x00,                  //8     LOCAL_USAGE_MINIMUM(0)
        0x2A,0x3C,0x03,             //10    LOCAL_USAGE_MAXIMUM(828)
        0x15,0x00,                  //13    GLOBAL_LOGICAL_MINIMUM(0)
        0x26,0x3C,0x03,             //15    GLOBAL_LOCAL_MAXIMUM(828)
        0x95,0x01,                  //18    GLOBAL_REPORT_COUNT(1)
        0x75,0x10,                  //20    GLOBAL_REPORT_SIZE(16)
        0x81,0x00,                  //22    MAIN_INPUT(data array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )    Input 2.0
    0xC0,                       //24    MAIN_COLLECTION_END

    0x05,0x01,                  //25    GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x80,                  //27    LOCAL_USAGE(SYS_CTL)
    0xA1,0x01,                  //29    MAIN_COLLECTION(Applicatior)
        0x85,REPORT_ID_SYS_CTL,                  //31    GLOBAL_REPORT_ID(2)
        0x05,0x01,                  //33    GLOBAL_USAGE_PAGE(Generic Desktop Controls)
        0x19,0x81,                  //35    LOCAL_USAGE_MINIMUM(129)
        0x29,0x83,                  //37    LOCAL_USAGE_MAXIMUM(131)
        0x15,0x00,                  //39    GLOBAL_LOGICAL_MINIMUM(0)
        0x25,0x01,                  //41    GLOBAL_LOCAL_MAXIMUM(1)
        0x95,0x03,                  //43    GLOBAL_REPORT_COUNT(3)
        0x75,0x01,                  //45    GLOBAL_REPORT_SIZE(1)
        0x81,0x02,                  //47    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 2.3
        0x95,0x01,                  //49    GLOBAL_REPORT_COUNT(1)
        0x75,0x05,                  //51    GLOBAL_REPORT_SIZE(5)
        0x81,0x01,                  //53    MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )   Input 3.0
    0xC0,                       //55    MAIN_COLLECTION_END

    0x05,0x01,                  //56    GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x06,                  //58    LOCAL_USAGE(Keyboard)
    0xA1,0x01,                  //60    MAIN_COLLECTION(Applicatior)
        0x85,REPORT_ID_ALL_KEYBOARD,                  //62    GLOBAL_REPORT_ID(1)
        0x05,0x07,                  //64    GLOBAL_USAGE_PAGE(Keyboard/Keypad)
        0x15,0x00,                  //66    GLOBAL_LOGICAL_MINIMUM(0)
        0x25,0x01,                  //68    GLOBAL_LOCAL_MAXIMUM(1)
        0x19,0x00,                  //70    LOCAL_USAGE_MINIMUM(0)
        0x29,0x77,                  //72    LOCAL_USAGE_MAXIMUM(119)
        0x95,0x78,                  //74    GLOBAL_REPORT_COUNT(120)
        0x75,0x01,                  //76    GLOBAL_REPORT_SIZE(1)
        0x81,0x02,                  //78    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 18.0
    0xC0,                       //80    MAIN_COLLECTION_END

};

//   -----报告描述符Manufacturer--------------
const uint8_t HID_ReportDescriptorManufacturer[]=
{
    0x06, 0x60, 0xFF,  // Usage Page (Vendor Defined 0xFF60)
    0x09, 0x61,        // Usage (0x61)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x62,        //   Usage (0x62)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x20,        //   Report Count (32)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x63,        //   Usage (0x63)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x20,        //   Report Count (32)
    0x75, 0x08,        //   Report Size (8)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};

//   -----报告描述符IAP--------------
const uint8_t HID_ReportDescriptorIAP[]=
{

    0x06, 0x70, 0xFF,  // Usage Page (Vendor Defined 0xFF60)
    0x09, 0x71,        // Usage (0x61)
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
const uint8_t HID_ReportDescSizeKeyboard = sizeof(HID_ReportDescriptorKeyboard);
const uint8_t HID_ReportDescSizeMouse = sizeof(HID_ReportDescriptorMouse);
const uint8_t HID_ReportDescSizeOther = sizeof(HID_ReportDescriptorOther);
const uint8_t HID_ReportDescSizeManufacturer = sizeof(HID_ReportDescriptorManufacturer);
const uint8_t HID_ReportDescSizeIAP = sizeof(HID_ReportDescriptorIAP);

// 设备描述符
uint8_t MyDevDescr[] = {
    0x12,   /* bLength */
    0x01,   /* bDescriptorType USB_DEVICE_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0200),  /* bcdUSB */
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
    USB_WBVAL(9+9+9+7+9+9+7+9+9+7+9+9+7+7),   /* wTotalLength */
    USB_INTERFACE_MAX_NUM,   /* bNumInterfaces */
    0x01,   /* bConfigurationValue */
    0x00,   /* iConfiguration */
    0xA0,
    0x32,    /* bMaxPower */
    //配置描述符

//   -----Interface 描述符 Keyboard --------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x00,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x01,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    //接口描述符

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0111),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeKeyboard),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x81,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP1 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP1SIZE),/* wMaxPacketSize */
    0x08,   /* bInterval *//* 1ms */
    //端点1描述符

//   -----Interface 描述符 Mouse --------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x01,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    //接口描述符

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0111),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeMouse),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x82,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP2 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP2SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 0.125ms */
    //端点2描述符

//   -----Interface 描述符 Other --------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x02,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x00,   /* bInterfaceSubClass */
    0x00,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    //接口描述符

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0111),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeOther),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x83,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP3 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP3SIZE),/* wMaxPacketSize */
    0x08,   /* bInterval *//* 1ms */

//   -----Interface 描述符 IAP --------------
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
    USB_WBVAL(0x0111),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeIAP),/* wDescriptorLength */
    //HID类描述符

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x84,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP4 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP4SIZE),/* wMaxPacketSize */
    0x08,   /* bInterval *//* 1ms */

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x04,   /* bEndpointAddress *//* USB_ENDPOINT_OUT USB_EP4 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP4SIZE),/* wMaxPacketSize */
    0x08,   /* bInterval *//* 1ms */
    //端点4描述符

};

uint8_t usb_desc_index = 0;
volatile uint8_t uart_wait_ack_falg = 0;
volatile uint32_t uart_trans_rtc = 0;
uint32_t rtc_count;
uint32_t usb_enum_success_flag=0;
uint8_t rf_data_buf[64] = {0};

/*******************************************************************************
 * @fn      app_relay_rf_to_uart
 *
 * @brief   RF收到数据后调用此函数发往USB
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void app_relay_rf_to_uart( )
{
    if(!uart_wait_ack_falg)
    {
        uint8_t *pPdu;
        uint8_t len;
        uint8_t state=STATE_SUCCESS;
        pPdu = rf_get_data(&len);
        if(pPdu)
        {
            if(usb_enum_success_flag)
            {
                switch(pPdu[0])
                {
                    case CMD_CLASS_KEYBOARD:
                    {
                        state = access_send_endp_data( ENDP_1, &pPdu[1], len-1);
                        break;
                    }
                    case CMD_MOUSE:
                    {
                        state = access_send_endp_data( ENDP_2, &pPdu[1], len-1);
                        if(state == STATE_SUCCESS )
                        {
                            gTxCount++;
                        }
                        break;
                    }
                    case CMD_ALL_KEYBOARD:
                    case CMD_CONSUMER:
                    case CMD_SYS_CTL:
                    {
                        state = access_send_endp_data( ENDP_3, &pPdu[1], len-1);
                        break;
                    }
                    case RF_DATA_IAP:
                    {
                        uint8_t pData[DevEP4SIZE]={0};
                        tmos_memcpy(pData, &pPdu[1], len-1);
                        state = access_send_endp_data( ENDP_4, pData, DevEP4SIZE);
                        break;
                    }
                }
                if(state == STATE_SUCCESS )
                {
                    uart_trans_rtc = RTC_GetCycle32k();
                    uart_wait_ack_falg = 1;
                    rf_delete_data();
                }
            }
        }
    }
}

/*******************************************************************************
 * @fn      app_relay_ota_to_uart
 *
 * @brief   ota需要发送数据调用此函数发往USB
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void app_relay_ota_to_uart( )
{
    if(!uart_wait_ack_falg)
    {
        if(ota_send_buf[0])
        {
            uint8_t state=STATE_SUCCESS;
            if(usb_enum_success_flag)
            {
                state = access_send_endp_data( ENDP_4, ota_send_buf, DevEP4SIZE);
                if(state == STATE_SUCCESS )
                {
                    uart_trans_rtc = RTC_GetCycle32k();
                    uart_wait_ack_falg = 1;
                    ota_send_buf[0] = 0;
                }
            }
        }
    }
}

/*******************************************************************************
 * @fn      app_retran_data_to_uart
 *
 * @brief   间隔RTC时钟重传上一包未应答数据
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void app_retran_data_to_uart( )
{
    if(uart_wait_ack_falg)
    {
        rtc_count = RTC_GetCycle32k();
        // 间隔5个RTC时钟重传
        if(RTC_A_SUB_B(rtc_count,uart_trans_rtc) > RETRAN_INTERVAL)
        {
            trans_retran_last_data();
            uart_trans_rtc = rtc_count;
        }
    }
}

/*******************************************************************************
 * @fn      app_data_process
 *
 * @brief   轮询是否有需要处理的数据
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void app_data_process( )
{
    app_relay_ota_to_uart();
    app_relay_rf_to_uart();
}

/*********************************************************************
 * @fn      app_cmd_cb
 *
 * @brief   app_cmd_cb
 *
 * @return  none
 */
__HIGH_CODE
void app_cmd_cb(uint8_t state, uint8_t cmd, uint8_t *pData, uint16_t len)
{
    if(!state)
    {
        switch(cmd&CMD_MRAK_MAX)
        {
            case CMD_SNED_ENDP_DATA1|CMD_ACK:
            {
                // 0x00表示执行成功，其它值表示失败。
                if(!pData[0])
                {
                    uart_wait_ack_falg = 0;
                }
                break;
            }

            case CMD_AUTO_SEND_STATUS:
            {
                PRINT("AUTO_STATUS %x\n",pData[0]);
                access_send_ack(CMD_AUTO_SEND_STATUS);
                if(pData[0] == STATUS_ENUM_SUCCESS)
                {
                    usb_enum_success_flag = 1;
                }
                else if(pData[0] == STATUS_SLEEP)
                {
                    if(pData[1] == ENTER_SLEEP)
                    {
                        // 发送给2.4G对端
                        rf_data_buf[0] = RF_DATA_SLEEP;
                        tmos_memcpy(&rf_data_buf[1], pData, len);
                        if(rf_send_data(rf_data_buf, len+1))
                        {
                            PRINT("rf_send pending \n");
                            break;
                        }
                    }
                    else if(pData[1] == EXIT_SLEEP){

                    }
                }
                break;
            }

            case CMD_AUTO_SEND_PC_DATA:
            {
                PRINT("PC_DATA ENDP %x\n",pData[0]);
                access_send_ack(cmd);
                switch((cmd&CMD_MRAK_ENDP)>>4)
                {
                    case ENDP_0:
                    {
                        rf_data_buf[0] = RF_DATA_LED;
                        tmos_memcpy(&rf_data_buf[1], pData, len);
                        if(rf_send_data(rf_data_buf, len+1))
                        {
                            PRINT("rf_send pending \n");
                            return;
                        }
                        break;
                    }

                    case ENDP_4:
                    {
                        // 自定义端点，检查是否是OTA等配置数据
                        if(OTA_IAPWriteData(&pData[0], len)==HEX_NOT_LOCAL)
                        {
                            // 说明需要发送给2.4G对端
                            rf_data_buf[0] = RF_DATA_IAP;
                            tmos_memcpy(&rf_data_buf[1], pData, len);
                            if(rf_send_data(rf_data_buf, len+1))
                            {
                                PRINT("rf_send pending \n");
                                return;
                            }
                        }
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
                break;
            }

            case CMD_CHK_CONNECT|CMD_ACK:
            {
                PRINT("CHK SUCCESS\n");
                access_get_info();
                break;
            }

            case CMD_GET_INFO|CMD_ACK:
            {
                PRINT("var %x\n",pData[0]);
                //0x00表示USB未连接；
                //0x01表示USB已连接但未枚举；
                //0x02表示USB已连接且枚举;
                PRINT("usb state %x\n",pData[1]);
                PRINT("usb SetReport %x\n",pData[2]);
                PRINT("io dir %x\n",pData[3]);
                PRINT("io pin %x\n",pData[4]);
                PRINT("buffused %x\n",pData[5]|(pData[6]<<8));
                if(pData[1] != 0x02)
                {
                    //USB设备描述符
                    access_set_usb_desc(usb_desc_index, 0, MyDevDescr[0], MyDevDescr);
                }
                break;
            }

            case CMD_SET_INFO|CMD_ACK:
            {
                PRINT("SET_INFO state %x\n",pData[0]);
                break;
            }

            case CMD_SET_USB_DESC|CMD_ACK:
            {
                PRINT("SET_USB_DESC %x state %x\n",usb_desc_index,pData[0]);
                if(!pData[0])
                {
                    usb_desc_index++;
                    switch(usb_desc_index)
                    {
                        case 1: //USB配置描述符
                        {
                            access_set_usb_desc(usb_desc_index, 0, MyCfgDescr[2], (uint8_t *)MyCfgDescr);
                            break;
                        }
                        case 2: //USB HID1报表描述符
                        {
                            access_set_usb_desc(usb_desc_index, 0, HID_ReportDescSizeKeyboard, (uint8_t *)HID_ReportDescriptorKeyboard);
                            break;
                        }
                        case 3: //USB HID2报表描述符
                        {
                            access_set_usb_desc(usb_desc_index, 0, HID_ReportDescSizeMouse, (uint8_t *)HID_ReportDescriptorMouse);
                            break;
                        }
                        case 4: //USB HID3报表描述符
                        {
                            access_set_usb_desc(usb_desc_index, 0, HID_ReportDescSizeOther, (uint8_t *)HID_ReportDescriptorOther);
                            break;
                        }
                        case 5: //USB HID4报表描述符
                        {
                            access_set_usb_desc(usb_desc_index, 0, HID_ReportDescSizeIAP, (uint8_t *)HID_ReportDescriptorIAP);
                            break;
                        }
                        case 6: //USB字符串0(语言)描述符
                        {
                            access_set_usb_desc(usb_desc_index+1, 0, sizeof(LangID_StrDescr), (uint8_t *)LangID_StrDescr);
                            break;
                        }
                        case 7: //USB字符串1(厂商)描述符
                        {
                            access_set_usb_desc(usb_desc_index+1, 0, sizeof(MyManuInfo), MyManuInfo);
                            break;
                        }
                        case 8: //USB字符串2(产品)描述符
                        {
                            access_set_usb_desc(usb_desc_index+1, 0, sizeof(MyProdInfo), MyProdInfo);
                            break;
                        }
                        case 9: //USB字符串3(序列号)描述符
                        {
                            access_set_usb_desc(usb_desc_index+1, 0, sizeof(MySerialNum), MySerialNum);
                            break;
                        }
                        case 0x0A: //USB字符串4描述符
                        {
                            // 无字符串4，直接启动USB
                            access_set_info(0x01, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x00);
                            break;
                        }
                    }
                }
                break;
            }

        }
    }
    else {
        PRINT("cmd tomeout %x\n",cmd);
    }
}


/*********************************************************************
 * @fn      app_ProcessEvent
 *
 * @brief   RF 事件处理
 *
 * @param   task_id - 任务ID
 * @param   events  - 事件标志
 *
 * @return  未完成事件
 */
uint16_t app_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if( events & 1 )
    {
        return (events ^ 1);
    }

    return 0;
}


/*********************************************************************
 * @fn      app_Init
 *
 * @brief   app_Init
 *
 * @return  none
 */
void app_Init( void )
{
    PRINT( "%s\n", VER_CH9160_LIB );
    access_register_cmd_cb(app_cmd_cb);
    app_taskID = TMOS_ProcessEventRegister(app_ProcessEvent);

    PRINT("app_Init\n");
    access_chk_connect();
}

void SoftwareUART_SendChar(char c)
{
  // 起始位（低电平）
  GPIOA_ResetBits(GPIO_Pin_12);
  DelayUs(104);

  // 数据位（低位在前）
  for (int i = 0; i < 8; i++)
  {
    if (c & (1 << i))
    {
        GPIOA_SetBits(GPIO_Pin_12);
    }
    else
    {
        GPIOA_ResetBits(GPIO_Pin_12);
    }
    DelayUs(104);
  }

  // 停止位（高电平）
  GPIOA_SetBits(GPIO_Pin_12);
  DelayUs(104);
}

void SoftwareUART_SendString(char* str)
{
  while (*str)
  {
    SoftwareUART_SendChar(*str++);
  }
}

void SoftwareUART_Printf(const char* format, ...)
{
    char buffer[1024]; // 定义一个足够大的缓冲区
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    SoftwareUART_SendString(buffer);
}

/******************************** endfile @ main ******************************/
