/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbdservice.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : ¼üÅÌ·þÎñ
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "hidkbdservice.h"
#include "hiddev.h"
#include "battservice.h"

/*********************************************************************
 * MACROS
 */
#define USE_SYS_CTR_HID 1
/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// HID service
const uint8_t hidServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(HID_SERV_UUID), HI_UINT16(HID_SERV_UUID)};

// HID Boot Keyboard Input Report characteristic
const uint8_t hidBootKeyInputUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BOOT_KEY_INPUT_UUID), HI_UINT16(BOOT_KEY_INPUT_UUID)};

// HID Boot Keyboard Output Report characteristic
const uint8_t hidBootKeyOutputUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BOOT_KEY_OUTPUT_UUID), HI_UINT16(BOOT_KEY_OUTPUT_UUID)};

// HID Information characteristic
const uint8_t hidInfoUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(HID_INFORMATION_UUID), HI_UINT16(HID_INFORMATION_UUID)};

// HID Report Map characteristic
const uint8_t hidReportMapUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(REPORT_MAP_UUID), HI_UINT16(REPORT_MAP_UUID)};

// HID Control Point characteristic
const uint8_t hidControlPointUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(HID_CTRL_PT_UUID), HI_UINT16(HID_CTRL_PT_UUID)};

// HID Report characteristic
const uint8_t hidReportUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(REPORT_UUID), HI_UINT16(REPORT_UUID)};

// HID Protocol Mode characteristic
const uint8_t hidProtocolModeUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(PROTOCOL_MODE_UUID), HI_UINT16(PROTOCOL_MODE_UUID)};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// HID Information characteristic value
static const uint8_t hidInfo[HID_INFORMATION_LEN] = {
    LO_UINT16(0x0111), HI_UINT16(0x0111), // bcdHID (USB HID version)
    0x00,                                 // bCountryCode
    HID_FEATURE_FLAGS                     // Flags
};
#define APPLE   0
//#define ENABLE_MAC_FN_REPORT   1
#define MOUSE   1
//#define SMART_WHEEL   1


// HID Report Map characteristic value
static const uint8_t hidReportMap[] = {

        0x05, 0x01,     // Usage Pg (Generic Desktop)
        0x09, 0x06,     // Usage (Keyboard)
        0xA1, 0x01,     // Collection: (Application)
        0x85, HID_RPT_ID_CLASS_KEY_IN,     //   ReportID(1)
                        //
        0x05, 0x07,     // Usage Pg (Key Codes)
        0x19, 0xE0,     // Usage Min (224)
        0x29, 0xE7,     // Usage Max (231)
        0x15, 0x00,     // Log Min (0)
        0x25, 0x01,     // Log Max (1)
                        //
                        // Modifier byte
        0x75, 0x01,     // Report Size (1)
        0x95, 0x08,     // Report Count (8)
        0x81, 0x02,     // Input: (Data, Variable, Absolute)
                        //
                        // Reserved byte
        0x95, 0x01,     // Report Count (1)
        0x75, 0x08,     // Report Size (8)
        0x81, 0x01,     // Input: (Constant)
                        //
                        // LED report
        0x95, 0x05,     // Report Count (5)
        0x75, 0x01,     // Report Size (1)
        0x05, 0x08,     // Usage Pg (LEDs)
        0x19, 0x01,     // Usage Min (1)
        0x29, 0x05,     // Usage Max (5)
        0x91, 0x02,     // Output: (Data, Variable, Absolute)
                        //
                        // LED report padding
        0x95, 0x01,     // Report Count (1)
        0x75, 0x03,     // Report Size (3)
        0x91, 0x01,     // Output: (Constant)
                        //
                        // Key arrays (6 bytes)
        0x95, 0x06,     // Report Count (6)
        0x75, 0x08,     // Report Size (8)
        0x15, 0x00,     // Log Min (0)
        0x25, 0x65,     // Log Max (101)
        0x05, 0x07,     // Usage Pg (Key Codes)
        0x19, 0x00,     // Usage Min (0)
        0x29, 0x65,     // Usage Max (101)
        0x81, 0x00,     // Input: (Data, Array)
                        //
        0xC0 ,           // End Collection
#if USE_SYS_CTR_HID
    0x05,0x01,                  //25    GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x80,                  //27    LOCAL_USAGE(SYS_CTL)
    0xA1,0x01,                  //29    MAIN_COLLECTION(Applicatior)
        0x85,HID_RPT_ID_SYS_CTL_IN,                  //31    GLOBAL_REPORT_ID(2)
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

//    //system control
//    0x05, 0x01,
//    0x09, 0x80,
//    0xA1, 0x01,
//    0x85, HID_RPT_ID_SYS_CTL_IN,  //report ID (2)
//    0x75, 0x02,
//    0x95, 0x01,
//    0x15, 0x01,
//    0x25, 0x03,
//    0x09, 0x82, //count 3
//    0x09, 0x81,
//    0x09, 0x83,
//    0x81, 0x60,
//    0x75, 0x06,
//    0x81, 0x03,
//    0xc0,

//    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
//    0x09, 0x80,        // Usage (Sys Control)
//    0xA1, 0x01,        // Collection (Application)
//    0x85, HID_RPT_ID_SYS_CTL_IN,        //   Report ID (2)
//    0x75, 0x01,        //   Report Size (1)
//    0x95, 0x08,        //   Report Count (8)
//    0x15, 0x00,        //   Logical Minimum (0)
//    0x25, 0x01,        //   Logical Maximum (1)
//    0x09, 0x81,        //   Usage (Sys Power Down)
//    0x09, 0x82,        //   Usage (Sys Sleep)
//    0x09, 0x83,        //   Usage (Sys Wake Up)
//    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
//    0xC0,              // End Collection

#if MOUSE
        0x05,0x01,                  //81    GLOBAL_USAGE_PAGE(Generic Desktop Controls)
        0x09,0x02,                  //83    LOCAL_USAGE(Mouse)
        0xA1,0x01,                  //85    MAIN_COLLECTION(Applicatior)
        0x09,0x01,                  //89    LOCAL_USAGE(Pointer)
        0xA1,0x00,                  //91    MAIN_COLLECTION(Physical)
        0x85,HID_RPT_ID_MOUSE_IN,                  //6     GLOBAL_REPORT_ID(3)
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
        0x16, 0x00, 0x80,  //     Logical Minimum (-32768)
        0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
        0x75,0x10,                  //127   GLOBAL_REPORT_SIZE(16)
        0x95,0x02,                  //129   GLOBAL_REPORT_COUNT(2)
        0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
        0x09,0x38,                  //121   LOCAL_USAGE(Wheel)
        0x15,0x81,                  //123   GLOBAL_LOGICAL_MINIMUM(-127)
        0x25,0x7F,                  //125   GLOBAL_LOCAL_MAXIMUM(127)
        0x75,0x08,                  //127   GLOBAL_REPORT_SIZE(8)
        0x95,0x01,                  //129   GLOBAL_REPORT_COUNT(1)
        0x81,0x06,                  //131   MAIN_INPUT(data var relative NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 22.0
        0x95, 0x01,        //     Report Count (1)
        0x05, 0x0C,        //     Usage Page (Consumer)
        0x0A, 0x38, 0x02,  //     Usage (AC Pan)
        0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,                       //133   MAIN_COLLECTION_END
        0xC0,                       //134   MAIN_COLLECTION_END


#endif

    0x05,0x0C,                  //0     GLOBAL_USAGE_PAGE(Consumer)
    0x09,0x01,                  //2     LOCAL_USAGE(    Consumer Control    )
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
        0x85,HID_RPT_ID_CONSUMER_IN,                  //6     GLOBAL_REPORT_ID(3)
        0x19,0x00,                  //8     LOCAL_USAGE_MINIMUM(0)
        0x2A,0x3C,0x03,             //10    LOCAL_USAGE_MAXIMUM(828)
        0x15,0x00,                  //13    GLOBAL_LOGICAL_MINIMUM(0)
        0x26,0x3C,0x03,             //15    GLOBAL_LOCAL_MAXIMUM(828)
        0x95,0x01,                  //18    GLOBAL_REPORT_COUNT(1)
        0x75,0x10,                  //20    GLOBAL_REPORT_SIZE(16)
        0x81,0x00,                  //22    MAIN_INPUT(data array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )    Input 2.0
    0xC0,                       //24    MAIN_COLLECTION_END

    0x05,0x01,                  //0     GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x06,                  //2     LOCAL_USAGE(Keyboard)
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
    0x85,HID_RPT_ID_ALL_KEY_IN,                  //6     GLOBAL_REPORT_ID(2)
    0x05,0x07,                  //8     GLOBAL_USAGE_PAGE(Keyboard/Keypad)
    0x19,0x04,                  //10    LOCAL_USAGE_MINIMUM(4)
    0x29,0x70,                  //12    LOCAL_USAGE_MAXIMUM(112)
    0x15,0x00,                  //14    GLOBAL_LOGICAL_MINIMUM(0)
    0x25,0x01,                  //16    GLOBAL_LOCAL_MAXIMUM(1)
    0x75,0x01,                  //18    GLOBAL_REPORT_SIZE(1)
    0x95,0x78,                  //20    GLOBAL_REPORT_COUNT(120)
    0x81,0x02,                  //22    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 15.0
    0xC0,                       //24    MAIN_COLLECTION_END

#ifdef  ENABLE_MAC_FN_REPORT
     0x05, 0x0c,   //# Usage Page (Consumer Devices)
     0x09, 0x01,   //#   Usage (Consumer Control)
     0xa1, 0x01,   //#   Collection (Application)
     0x85, HID_RPT_ID_FN_IN, //  ###~~~~~~~~~~~~~  #   Report ID=0x11
     0x15, 0x00,   //#LOGICAL_MINIMUM(0)
     0x25, 0x01,   //#LOGICAL_MAXIMUM(1)
     0x75, 0x01,   //# REPORT_SIZE(1)
     0x95, 0x03,   //#REPORT_COUNT(3)
     0x81, 0x01,   //#INPUT(Const,Ary,Abs)-------------3bit
     0x75, 0x01,   //#REPORT_SIZE(1)
     0x95, 0x01,   //#REPORT_COUNT(1)
     0x05, 0x0c,   //#USAGE_PAGE(Consumer Devices)
     0x09, 0xb8,   //#USAGE(Eject)
     0x81, 0x02,   //#INPUT(Data,Var,Abs)----------------1bit
     0x06, 0xff, 0x00,   //#USAGE_PAGE(Reserved)
     0x09, 0x03,   //#USAGE(Fn)
     0x81, 0x02,   //#INPUT(Data,Var,Abs)----------------1bit
     0x75, 0x01,   //#REPORT_SIZE(1)
     0x95, 0x03,   //#REPORT_COUNT(3)
     0x81, 0x01,   //#INPUT(Const,Ary,Abs)----------------3bit
     0xc0,
#endif
#ifdef  SMART_WHEEL
     // smart wheel
     0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
     0x09, 0x0E,        // Usage (0x0E)
     0xA1, 0x01,        // Collection (Application)
     0x85, HID_RPT_ID_SMART_WHEEL_IN,        //   Report ID (8)
     0x05, 0x0D,        //   Usage Page (Digitizer)
     0x09, 0x21,        //   Usage (Puck)
     0xA1, 0x00,        //   Collection (Physical)
     0x05, 0x09,        //     Usage Page (Button)
     0x09, 0x01,        //     Usage (0x01)
     0x95, 0x01,        //     Report Count (1)
     0x75, 0x01,        //     Report Size (1)
     0x15, 0x00,        //     Logical Minimum (0)
     0x25, 0x01,        //     Logical Maximum (1)
     0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
     0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
     0x09, 0x37,        //     Usage (Dial)
     0x95, 0x01,        //     Report Count (1)
     0x75, 0x0F,        //     Report Size (15)
     0x55, 0x0F,        //     Unit Exponent (-1)
     0x65, 0x14,        //     Unit (System: English Rotation, Length: Centimeter)
     0x36, 0xF0, 0xF1,  //     Physical Minimum (-3600)
     0x46, 0x10, 0x0E,  //     Physical Maximum (3600)
     0x16, 0xF0, 0xF1,  //     Logical Minimum (-3600)
     0x26, 0x10, 0x0E,  //     Logical Maximum (3600)
     0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
     0x09, 0x30,        //     Usage (X)
     0x75, 0x10,        //     Report Size (16)
     0x55, 0x0D,        //     Unit Exponent (-3)
     0x65, 0x13,        //     Unit (System: English Linear, Length: Centimeter)
     0x35, 0x00,        //     Physical Minimum (0)
     0x46, 0xC0, 0x5D,  //     Physical Maximum (24000)
     0x15, 0x00,        //     Logical Minimum (0)
     0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
     0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
     0x09, 0x31,        //     Usage (Y)
     0x46, 0xB0, 0x36,  //     Physical Maximum (14000)
     0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
     0x05, 0x0D,        //     Usage Page (Digitizer)
     0x09, 0x48,        //     Usage (0x48)
     0x36, 0xB8, 0x0B,  //     Physical Minimum (3000)
     0x46, 0xB8, 0x0B,  //     Physical Maximum (3000)
     0x16, 0xB8, 0x0B,  //     Logical Minimum (3000)
     0x26, 0xB8, 0x0B,  //     Logical Maximum (3000)
     0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
     0xC0,              //   End Collection
     0xC0,              // End Collection
#endif
     // 101 bytes
//    0x06,0xFF,0xFF,             //135   GLOBAL_USAGE_PAGE(Reserved or Other)
//    0x09,0x01,                  //138   LOCAL_USAGE()
//    0xA1,0x01,                  //140   MAIN_COLLECTION(Applicatior)
//        0x85,HID_RPT_ID_OTHER,                  //142   GLOBAL_REPORT_ID(5)
//        0x09,0x01,                  //144   LOCAL_USAGE()
//        0x15,0x00,                  //146   GLOBAL_LOGICAL_MINIMUM(0)
//        0x26,0xFF,0x00,             //148   GLOBAL_LOCAL_MAXIMUM(255/255)
//        0x75,0x08,                  //151   GLOBAL_REPORT_SIZE(8)
//        0x95,0x03,                  //153   GLOBAL_REPORT_COUNT(3)
//        0x81,0x02,                  //155   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 25.0
//    0xC0,                       //157   MAIN_COLLECTION_END
#endif
};

// HID report map length
uint16_t hidReportMapLen = sizeof(hidReportMap);

// HID report mapping table
static hidRptMap_t hidRptMap[HID_NUM_REPORTS];

/*********************************************************************
 * Profile Attributes - variables
 */

// HID Service attribute
static const gattAttrType_t hidService = {ATT_BT_UUID_SIZE, hidServUUID};

// Include attribute (Battery service)
static uint16_t include = GATT_INVALID_HANDLE;

// HID Information characteristic
static uint8_t hidInfoProps = GATT_PROP_READ;

// HID Report Map characteristic
static uint8_t hidReportMapProps = GATT_PROP_READ;

// HID External Report Reference Descriptor
static uint8_t hidExtReportRefDesc[ATT_BT_UUID_SIZE] =
    {LO_UINT16(BATT_LEVEL_UUID), HI_UINT16(BATT_LEVEL_UUID)};

// HID Control Point characteristic
static uint8_t hidControlPointProps = GATT_PROP_WRITE_NO_RSP;
static uint8_t hidControlPoint;

// HID Protocol Mode characteristic
static uint8_t hidProtocolModeProps = GATT_PROP_READ | GATT_PROP_WRITE_NO_RSP;
uint8_t        hidProtocolMode = HID_PROTOCOL_MODE_REPORT;

// HID Report characteristic, class key input
static uint8_t       hidReportClassKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportClassKeyIn;
static gattCharCfg_t hidReportClassKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, class key input
static uint8_t hidReportRefClassKeyIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_CLASS_KEY_IN, HID_REPORT_TYPE_INPUT};

// HID Report characteristic, LED output
static uint8_t hidReportLedOutProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8_t hidReportLedOut;

// HID Report Reference characteristic descriptor, LED output
static uint8_t hidReportRefLedOut[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_LED_OUT, HID_REPORT_TYPE_OUTPUT};

// HID Report characteristic, Mouse input
static uint8_t       hidReportMouseInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportMouseIn;
static gattCharCfg_t hidReportMouseInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, Mouse input
static uint8_t hidReportRefMouseIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT};

// HID Report characteristic, consumer input
static uint8_t       hidReportConsumerInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportConsumerIn;
static gattCharCfg_t hidReportConsumerInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, consumer input
static uint8_t hidReportRefConsumerIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT};

#if USE_SYS_CTR_HID

// HID Report characteristic, sys_ctl input
static uint8_t       hidReportSysCtlInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportSysCtlIn;
static gattCharCfg_t hidReportSysCtlInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, sys_ctl input
static uint8_t hidReportSysCtlRefIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_SYS_CTL_IN, HID_REPORT_TYPE_INPUT};
#endif

// HID Report characteristic, ALL_KEY input
static uint8_t       hidReportAllKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportAllKeyIn;
static gattCharCfg_t hidReportAllKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, ALL_KEY input
static uint8_t hidReportAllKeyRefIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_ALL_KEY_IN, HID_REPORT_TYPE_INPUT};

// HID Report characteristic, Fn input
static uint8_t       hidReportFnInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportFnIn;
static gattCharCfg_t hidReportFnInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, Fn input
static uint8_t hidReportFnRefIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_FN_IN, HID_REPORT_TYPE_INPUT};

// HID Report characteristic, SmartWheel input
static uint8_t       hidReportSmartWheelInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportSmartWheelIn;
static gattCharCfg_t hidReportSmartWheelInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, SmartWheel input
static uint8_t hidReportSmartWheelRefIn[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_SMART_WHEEL_IN, HID_REPORT_TYPE_INPUT};

// HID Boot Keyboard Input Report
static uint8_t       hidReportBootKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8_t       hidReportBootKeyIn;
static gattCharCfg_t hidReportBootKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Boot Keyboard Output Report
static uint8_t hidReportBootKeyOutProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8_t hidReportBootKeyOut;

// Feature Report
static uint8_t hidReportFeatureProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint8_t hidReportFeature;

// HID Report Reference characteristic descriptor, Feature
static uint8_t hidReportRefFeature[HID_REPORT_REF_LEN] =
    {HID_RPT_ID_FEATURE, HID_REPORT_TYPE_FEATURE};

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t hidAttrTbl[] = {
    // HID Service
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                       /* permissions */
        0,                                      /* handle */
        (uint8_t *)&hidService                  /* pValue */
    },

    // Included service (battery)
    {
        {ATT_BT_UUID_SIZE, includeUUID},
        GATT_PERMIT_READ,
        0,
        (uint8_t *)&include},

    // HID Information characteristic declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidInfoProps},

    // HID Information characteristic
    {
        {ATT_BT_UUID_SIZE, hidInfoUUID},
        GATT_PERMIT_ENCRYPT_READ,
        0,
        (uint8_t *)hidInfo},

    // HID Control Point characteristic declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidControlPointProps},

    // HID Control Point characteristic
    {
        {ATT_BT_UUID_SIZE, hidControlPointUUID},
        GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidControlPoint},

    // HID Protocol Mode characteristic declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidProtocolModeProps},

    // HID Protocol Mode characteristic
    {
        {ATT_BT_UUID_SIZE, hidProtocolModeUUID},
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidProtocolMode},

    // HID Report Map characteristic declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportMapProps},

    // HID Report Map characteristic
    {
        {ATT_BT_UUID_SIZE, hidReportMapUUID},
        GATT_PERMIT_ENCRYPT_READ,
        0,
        (uint8_t *)hidReportMap},

    // HID External Report Reference Descriptor
    {
        {ATT_BT_UUID_SIZE, extReportRefUUID},
        GATT_PERMIT_READ,
        0,
        hidExtReportRefDesc},

#if USE_SYS_CTR_HID

    // HID Report characteristic, sys_ctl input declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportSysCtlInProps},

        // HID Report characteristic, sys_ctl input
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportSysCtlIn},

        // HID Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportSysCtlInClientCharCfg},

        // HID Report Reference characteristic descriptor, sys_ctl input
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportSysCtlRefIn},
#endif

    // HID Report characteristic, class key input declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportClassKeyInProps},

        // HID Report characteristic, class key input
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportClassKeyIn},

        // HID Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportClassKeyInClientCharCfg},

        // HID Report Reference characteristic descriptor, class key input
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportRefClassKeyIn},

    // HID Report characteristic, LED output declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportLedOutProps},

        // HID Report characteristic, LED output
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            &hidReportLedOut},

        // HID Report Reference characteristic descriptor, LED output
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportRefLedOut},

    // HID Report characteristic, Mouse input declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportMouseInProps},

        // HID Report characteristic, Mouse input
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportMouseIn},

        // HID Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportMouseInClientCharCfg},

        // HID Report Reference characteristic descriptor, Mouse input
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportRefMouseIn},

    // HID Report characteristic, consumer input declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportConsumerInProps},

        // HID Report characteristic, consumer input
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportConsumerIn},

        // HID Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportConsumerInClientCharCfg},

        // HID Report Reference characteristic descriptor, consumer input
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportRefConsumerIn},

    // HID Report characteristic, ALL_KEY input declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportAllKeyInProps},

        // HID Report characteristic, ALL_KEY input
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportAllKeyIn},

        // HID Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportAllKeyInClientCharCfg},

        // HID Report Reference characteristic descriptor, ALL_KEY input
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportAllKeyRefIn},

    // HID Report characteristic, Fn input declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportFnInProps},

        // HID Report characteristic, Fn input
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportFnIn},

        // HID Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportFnInClientCharCfg},

        // HID Report Reference characteristic descriptor, Fn input
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportFnRefIn},

    // HID Report characteristic, SmartWheel input declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportSmartWheelInProps},

        // HID Report characteristic, SmartWheel input
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportSmartWheelIn},

        // HID Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportSmartWheelInClientCharCfg},

        // HID Report Reference characteristic descriptor, SmartWheel input
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportSmartWheelRefIn},

    // HID Boot Keyboard Input Report declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportBootKeyInProps},

        // HID Boot Keyboard Input Report
        {
            {ATT_BT_UUID_SIZE, hidBootKeyInputUUID},
            GATT_PERMIT_ENCRYPT_READ,
            0,
            &hidReportBootKeyIn},

        // HID Boot Keyboard Input Report characteristic client characteristic configuration
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            (uint8_t *)&hidReportBootKeyInClientCharCfg},

    // HID Boot Keyboard Output Report declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportBootKeyOutProps},

        // HID Boot Keyboard Output Report
        {
            {ATT_BT_UUID_SIZE, hidBootKeyOutputUUID},
            GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            &hidReportBootKeyOut},

    // Feature Report declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &hidReportFeatureProps},

        // Feature Report
        {
            {ATT_BT_UUID_SIZE, hidReportUUID},
            GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
            0,
            &hidReportFeature},

        // HID Report Reference characteristic descriptor, feature
        {
            {ATT_BT_UUID_SIZE, reportRefUUID},
            GATT_PERMIT_READ,
            0,
            hidReportRefFeature},
};

// Attribute index enumeration-- these indexes match array elements above
enum
{
    HID_SERVICE_IDX,             // HID Service
    HID_INCLUDED_SERVICE_IDX,    // Included Service
    HID_INFO_DECL_IDX,           // HID Information characteristic declaration
    HID_INFO_IDX,                // HID Information characteristic
    HID_CONTROL_POINT_DECL_IDX,  // HID Control Point characteristic declaration
    HID_CONTROL_POINT_IDX,       // HID Control Point characteristic
    HID_PROTOCOL_MODE_DECL_IDX,  // HID Protocol Mode characteristic declaration
    HID_PROTOCOL_MODE_IDX,       // HID Protocol Mode characteristic
    HID_REPORT_MAP_DECL_IDX,     // HID Report Map characteristic declaration
    HID_REPORT_MAP_IDX,          // HID Report Map characteristic
    HID_EXT_REPORT_REF_DESC_IDX, // HID External Report Reference Descriptor

#if USE_SYS_CTR_HID
    HID_REPORT_SYS_CTL_IN_DECL_IDX,  // HID Report characteristic, SYS_CTL input declaration
    HID_REPORT_SYS_CTL_IN_IDX,       // HID Report characteristic, SYS_CTL input
    HID_REPORT_SYS_CTL_IN_CCCD_IDX,  // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_SYS_CTL_IN_IDX,   // HID Report Reference characteristic descriptor, SYS_CTL input
#endif

    HID_REPORT_CLASS_KEY_IN_DECL_IDX,  // HID Report characteristic, CLASS key input declaration
    HID_REPORT_CLASS_KEY_IN_IDX,       // HID Report characteristic, CLASS key input
    HID_REPORT_CLASS_KEY_IN_CCCD_IDX,  // HID Report characteristic client characteristic configuration
    HID_REPORT_CLASS_REF_KEY_IN_IDX,   // HID Report Reference characteristic descriptor, CLASS key input

    HID_REPORT_LED_OUT_DECL_IDX, // HID Report characteristic, LED output declaration
    HID_REPORT_LED_OUT_IDX,      // HID Report characteristic, LED output
    HID_REPORT_REF_LED_OUT_IDX,  // HID Report Reference characteristic descriptor, LED output

    HID_REPORT_MOUSE_IN_DECL_IDX,  // HID Report characteristic, MOUSE input declaration
    HID_REPORT_MOUSE_IN_IDX,       // HID Report characteristic, MOUSE input
    HID_REPORT_MOUSE_IN_CCCD_IDX,  // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_MOUSE_IN_IDX,   // HID Report Reference characteristic descriptor, MOUSE input

    HID_REPORT_CONSUMER_IN_DECL_IDX,  // HID Report characteristic, CONSUMER input declaration
    HID_REPORT_CONSUMER_IN_IDX,       // HID Report characteristic, CONSUMER input
    HID_REPORT_CONSUMER_IN_CCCD_IDX,  // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_CONSUMER_IN_IDX,   // HID Report Reference characteristic descriptor, CONSUMER input

    HID_REPORT_ALL_KEY_IN_DECL_IDX,  // HID Report characteristic, ALL_KEY input declaration
    HID_REPORT_ALL_KEY_IN_IDX,       // HID Report characteristic, ALL_KEY input
    HID_REPORT_ALL_KEY_IN_CCCD_IDX,  // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_ALL_KEY_IN_IDX,   // HID Report Reference characteristic descriptor, ALL_KEY input

    HID_REPORT_FN_IN_DECL_IDX,  // HID Report characteristic, FN input declaration
    HID_REPORT_FN_IN_IDX,       // HID Report characteristic, FN input
    HID_REPORT_FN_IN_CCCD_IDX,  // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_FN_IN_IDX,   // HID Report Reference characteristic descriptor, FN input

    HID_REPORT_SMART_WHEEL_IN_DECL_IDX,  // HID Report characteristic, SMART_WHEEL input declaration
    HID_REPORT_SMART_WHEEL_IN_IDX,       // HID Report characteristic, SMART_WHEEL input
    HID_REPORT_SMART_WHEEL_IN_CCCD_IDX,  // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_SMART_WHEEL_IN_IDX,   // HID Report Reference characteristic descriptor, SMART_WHEEL input

    HID_BOOT_KEY_IN_DECL_IDX,    // HID Boot Keyboard Input Report declaration
    HID_BOOT_KEY_IN_IDX,         // HID Boot Keyboard Input Report
    HID_BOOT_KEY_IN_CCCD_IDX,    // HID Boot Keyboard Input Report characteristic client characteristic configuration
    HID_BOOT_KEY_OUT_DECL_IDX,   // HID Boot Keyboard Output Report declaration
    HID_BOOT_KEY_OUT_IDX,        // HID Boot Keyboard Output Report

    HID_FEATURE_DECL_IDX,        // Feature Report declaration
    HID_FEATURE_IDX,             // Feature Report
    HID_REPORT_REF_FEATURE_IDX   // HID Report Reference characteristic descriptor, feature
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Service Callbacks
gattServiceCBs_t hidKbdCBs = {
    HidDev_ReadAttrCB,  // Read callback function pointer
    HidDev_WriteAttrCB, // Write callback function pointer
    NULL                // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Hid_AddService
 *
 * @brief   Initializes the HID Service by registering
 *          GATT attributes with the GATT server.
 *
 * @return  Success or Failure
 */
bStatus_t Hid_AddService(void)
{
    uint8_t status = SUCCESS;

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportClassKeyInClientCharCfg);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportMouseInClientCharCfg);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportConsumerInClientCharCfg);
#if USE_SYS_CTR_HID
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportSysCtlInClientCharCfg);
#endif
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportAllKeyInClientCharCfg);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportFnInClientCharCfg);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportSmartWheelInClientCharCfg);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, hidReportBootKeyInClientCharCfg);

    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(hidAttrTbl, GATT_NUM_ATTRS(hidAttrTbl), GATT_MAX_ENCRYPT_KEY_SIZE, &hidKbdCBs);

    // Set up included service
    Batt_GetParameter(BATT_PARAM_SERVICE_HANDLE,
                      &GATT_INCLUDED_HANDLE(hidAttrTbl, HID_INCLUDED_SERVICE_IDX));

    // Construct map of reports to characteristic handles
    // Each report is uniquely identified via its ID and type

    // Class Key input report
    hidRptMap[0].id = hidReportRefClassKeyIn[0];
    hidRptMap[0].type = hidReportRefClassKeyIn[1];
    hidRptMap[0].handle = hidAttrTbl[HID_REPORT_CLASS_KEY_IN_IDX].handle;
    hidRptMap[0].cccdHandle = hidAttrTbl[HID_REPORT_CLASS_KEY_IN_CCCD_IDX].handle;
    hidRptMap[0].mode = HID_PROTOCOL_MODE_REPORT;
//    PRINT("%x \n",hidRptMap[0].cccdHandle);

    // LED output report
    hidRptMap[1].id = hidReportRefLedOut[0];
    hidRptMap[1].type = hidReportRefLedOut[1];
    hidRptMap[1].handle = hidAttrTbl[HID_REPORT_LED_OUT_IDX].handle;
    hidRptMap[1].cccdHandle = 0;
    hidRptMap[1].mode = HID_PROTOCOL_MODE_REPORT;

    // Mouse input report
    hidRptMap[2].id = hidReportRefMouseIn[0];
    hidRptMap[2].type = hidReportRefMouseIn[1];
    hidRptMap[2].handle = hidAttrTbl[HID_REPORT_MOUSE_IN_IDX].handle;
    hidRptMap[2].cccdHandle = hidAttrTbl[HID_REPORT_MOUSE_IN_CCCD_IDX].handle;
    hidRptMap[2].mode = HID_PROTOCOL_MODE_REPORT;
//    PRINT("%x \n",hidRptMap[0].cccdHandle);

    // consumer input report
    hidRptMap[3].id = hidReportRefConsumerIn[0];
    hidRptMap[3].type = hidReportRefConsumerIn[1];
    hidRptMap[3].handle = hidAttrTbl[HID_REPORT_CONSUMER_IN_IDX].handle;
    hidRptMap[3].cccdHandle = hidAttrTbl[HID_REPORT_CONSUMER_IN_CCCD_IDX].handle;
    hidRptMap[3].mode = HID_PROTOCOL_MODE_REPORT;
//    PRINT("%x \n",hidRptMap[3].cccdHandle);

#if USE_SYS_CTR_HID
    // sys_ctl input report
    hidRptMap[4].id = hidReportSysCtlRefIn[0];
    hidRptMap[4].type = hidReportSysCtlRefIn[1];
    hidRptMap[4].handle = hidAttrTbl[HID_REPORT_SYS_CTL_IN_IDX].handle;
    hidRptMap[4].cccdHandle = hidAttrTbl[HID_REPORT_SYS_CTL_IN_CCCD_IDX].handle;
    hidRptMap[4].mode = HID_PROTOCOL_MODE_REPORT;
//    PRINT("%x \n",hidRptMap[4].cccdHandle);
#endif

    // ALL_KEY input report
    hidRptMap[5].id = hidReportAllKeyRefIn[0];
    hidRptMap[5].type = hidReportAllKeyRefIn[1];
    hidRptMap[5].handle = hidAttrTbl[HID_REPORT_ALL_KEY_IN_IDX].handle;
    hidRptMap[5].cccdHandle = hidAttrTbl[HID_REPORT_ALL_KEY_IN_CCCD_IDX].handle;
    hidRptMap[5].mode = HID_PROTOCOL_MODE_REPORT;
//    PRINT("%x \n",hidRptMap[5].cccdHandle);

    // FN input report
    hidRptMap[6].id = hidReportFnRefIn[0];
    hidRptMap[6].type = hidReportFnRefIn[1];
    hidRptMap[6].handle = hidAttrTbl[HID_REPORT_FN_IN_IDX].handle;
    hidRptMap[6].cccdHandle = hidAttrTbl[HID_REPORT_FN_IN_CCCD_IDX].handle;
    hidRptMap[6].mode = HID_PROTOCOL_MODE_REPORT;
//    PRINT("%x \n",hidRptMap[6].cccdHandle);

    // SMART_WHEEL input report
    hidRptMap[7].id = hidReportSmartWheelRefIn[0];
    hidRptMap[7].type = hidReportSmartWheelRefIn[1];
    hidRptMap[7].handle = hidAttrTbl[HID_REPORT_SMART_WHEEL_IN_IDX].handle;
    hidRptMap[7].cccdHandle = hidAttrTbl[HID_REPORT_SMART_WHEEL_IN_CCCD_IDX].handle;
    hidRptMap[7].mode = HID_PROTOCOL_MODE_REPORT;
//    PRINT("%x \n",hidRptMap[6].cccdHandle);

    // Boot keyboard input report
    // Use same ID and type as key input report
    hidRptMap[8].id = hidReportRefClassKeyIn[0];
    hidRptMap[8].type = hidReportRefClassKeyIn[1];
    hidRptMap[8].handle = hidAttrTbl[HID_BOOT_KEY_IN_IDX].handle;
    hidRptMap[8].cccdHandle = hidAttrTbl[HID_BOOT_KEY_IN_CCCD_IDX].handle;
    hidRptMap[8].mode = HID_PROTOCOL_MODE_BOOT;
//    PRINT("%x \n",hidRptMap[7].cccdHandle);

    // Boot keyboard output report
    // Use same ID and type as LED output report
    hidRptMap[9].id = hidReportRefLedOut[0];
    hidRptMap[9].type = hidReportRefLedOut[1];
    hidRptMap[9].handle = hidAttrTbl[HID_BOOT_KEY_OUT_IDX].handle;
    hidRptMap[9].cccdHandle = 0;
    hidRptMap[9].mode = HID_PROTOCOL_MODE_BOOT;

    // Feature report
    hidRptMap[10].id = hidReportRefFeature[0];
    hidRptMap[10].type = hidReportRefFeature[1];
    hidRptMap[10].handle = hidAttrTbl[HID_FEATURE_IDX].handle;
    hidRptMap[10].cccdHandle = 0;
    hidRptMap[10].mode = HID_PROTOCOL_MODE_REPORT;

    // Battery level input report
    Batt_GetParameter(BATT_PARAM_BATT_LEVEL_IN_REPORT, &(hidRptMap[11]));

    // Setup report ID map
    HidDev_RegisterReports(HID_NUM_REPORTS, hidRptMap);

    return (status);
}

/*********************************************************************
 * @fn      Hid_SetParameter
 *
 * @brief   Set a HID Kbd parameter.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   len - length of data to right.
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the input parameters and WILL be cast to the appropriate
 *          data type (example: data type of uint16_t will be cast to
 *          uint16_t pointer).
 *
 * @return  GATT status code.
 */
uint8_t Hid_SetParameter(uint8_t id, uint8_t type, uint16_t uuid, uint8_t len, void *pValue)
{
    bStatus_t ret = SUCCESS;

    switch(uuid)
    {
        case REPORT_UUID:
            if(type == HID_REPORT_TYPE_OUTPUT)
            {
                if(len == 1)
                {
                    hidReportLedOut = *((uint8_t *)pValue);
                }
                else
                {
                    ret = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else if(type == HID_REPORT_TYPE_FEATURE)
            {
                if(len == 1)
                {
                    hidReportFeature = *((uint8_t *)pValue);
                }
                else
                {
                    ret = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else
            {
                ret = ATT_ERR_ATTR_NOT_FOUND;
            }
            break;

        case BOOT_KEY_OUTPUT_UUID:
            if(len == 1)
            {
                hidReportBootKeyOut = *((uint8_t *)pValue);
            }
            else
            {
                ret = ATT_ERR_INVALID_VALUE_SIZE;
            }
            break;

        default:
            // ignore the request
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      Hid_GetParameter
 *
 * @brief   Get a HID Kbd parameter.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   pLen - length of data to be read
 * @param   pValue - pointer to data to get.  This is dependent on
 *          the input parameters and WILL be cast to the appropriate
 *          data type (example: data type of uint16_t will be cast to
 *          uint16_t pointer).
 *
 * @return  GATT status code.
 */
uint8_t Hid_GetParameter(uint8_t id, uint8_t type, uint16_t uuid, uint16_t *pLen, void *pValue)
{
    switch(uuid)
    {
        case REPORT_UUID:
            if(type == HID_REPORT_TYPE_OUTPUT)
            {
                *((uint8_t *)pValue) = hidReportLedOut;
                *pLen = 1;
            }
            else if(type == HID_REPORT_TYPE_FEATURE)
            {
                *((uint8_t *)pValue) = hidReportFeature;
                *pLen = 1;
            }
            else
            {
                *pLen = 0;
            }
            break;

        case BOOT_KEY_OUTPUT_UUID:
            *((uint8_t *)pValue) = hidReportBootKeyOut;
            *pLen = 1;
            break;

        default:
            *pLen = 0;
            break;
    }

    return (SUCCESS);
}

/*********************************************************************
*********************************************************************/
