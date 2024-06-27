/********************************** (C) COPYRIGHT *******************************
 * File Name          : app.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/11/12
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef app_H
#define app_H

#ifdef __cplusplus
extern "C" {
#endif

#define USB_VID     0x1A86
#define USB_PID     0x8300
#define USB_REV     0x0105

#define DevEP0SIZE    0x40
#define DevEP1SIZE    0x08
#define DevEP2SIZE    0x08
#define DevEP3SIZE    0x10
#define DevEP4SIZE    0x40

#define ENDP_0          0x00
#define ENDP_1          0x01
#define ENDP_2          0x02
#define ENDP_3          0x03
#define ENDP_4          0x04

#define USB_WBVAL(x) ((x) & 0xFF),(((x) >> 8) & 0xFF)

#define REPORT_ID_CONSUMER      0x01
#define REPORT_ID_SYS_CTL       0x02
#define REPORT_ID_ALL_KEYBOARD  0x03
#define REPORT_ID_MANUFACTURER  0x05
#define REPORT_ID_IAP           0x09

// RF基本数据
#define CMD_CLASS_KEYBOARD      0x81
#define CMD_ALL_KEYBOARD        0x82
#define CMD_CONSUMER            0x83
#define CMD_SYS_CTL             0x84
#define CMD_FN_DATA             0x85
#define CMD_MOUSE               0x86
#define CMD_BATT_INFO           0x87
#define CMD_MANUFACTURER        0x88
#define CMD_BLE_MANUFACTURER    0x89

#define RF_DATA_LED                 CMD_CLASS_KEYBOARD
#define RF_DATA_IAP                 0x92
#define RF_DATA_MANUFACTURER        CMD_MANUFACTURER
#define RF_DATA_SLEEP               0x93
#define RF_DATA_BLE_MANUFACTURER    CMD_BLE_MANUFACTURER

#define RTC_TIMER_MAX_VALUE    0xa8c00000

//__attribute__((always_inline))
RV_STATIC_INLINE uint32_t RTC_A_SUB_B(uint32_t a, uint32_t b)
{
    if(a>=b) return (a-b);
    else return(a+(RTC_TIMER_MAX_VALUE-b));
}


//__attribute__((always_inline))
RV_STATIC_INLINE uint32_t RTC_A_ADD_B(uint32_t a, uint32_t b)
{
    uint64_t temp;
    temp = (uint64_t)a + b;
    if(temp>=RTC_TIMER_MAX_VALUE)   temp-=RTC_TIMER_MAX_VALUE;
    return(temp);
}
void app_Init( void );
void app_relay_rf_to_uart(void );
void app_relay_ota_to_uart(void );
void app_retran_data_to_uart(void );
void app_data_process(void);
void SoftwareUART_Printf(const char* format, ...);
void SoftwareUART_SendString(char* str);
#ifdef __cplusplus
}
#endif

#endif
