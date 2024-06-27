/********************************** (C) COPYRIGHT *******************************
* File Name          : mouse.h
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/10
* Description        : 
*******************************************************************************/

#ifndef __mouse_H
#define __mouse_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "CH59x_common.h"

// 电池电量采集间隔
#define GET_VBAT_INFO_INTERVAL            1600*5

#define MOUSE_LED_BLINK_EVENT            1<<0
#define MOUSE_LED_TIMEOUT_EVENT            1<<1
#define MOUSE_VBAT_INFO_EVENT            1<<2
#define MOUSE_LED_OFF_EVENT            1<<3
#define IDEL_SLEEP_EVENT            1<<8
#define DEEP_SLEEP_EVENT            1<<9

typedef enum
{
    CON_IDEL = 0,
    CON_CONNECTING,
    CON_NEW_PAIRING,
    CON_CONNECTED,
}connect_status_t;

#define PWM_RAM_SAFE_FLAG       0x30de5820
#define R32_PWM_RAM_SAFE_FLAG   (*((PUINT32V)0x20000000))
#define R32_PWM_CONTROL_RAM     (*((PUINT32V)0x20000004))
#define R32_PWM4_7_DATA_RAM     (*((PUINT32V)0x20000008)) // RW, PWM4-7 data holding
#define R32_PWM8_11_DATA_RAM    (*((PUINT32V)0x2000000C)) // RW, PWM8-11 data holding

#define VDR_R              (GPIO_Pin_7)       //B
#define VDR_G              (GPIO_Pin_23)       //
#define VDR_B              (GPIO_Pin_4)       //

#define VDR_PWM_R              (CH_PWM9)       //
#define VDR_PWM_G              (CH_PWM11)       //
#define VDR_PWM_B              (CH_PWM7)       //

#define LED_BLINK_ALL             (VDR_PWM_R|VDR_PWM_G|VDR_PWM_B)

extern connect_status_t connect_state;
extern uint8_t MOUSE_taskID;
extern uint32_t pilot_led_states;

#define PWMX_LED_ALL_OFF()    PWMX_ACTOUT(LED_BLINK_ALL, nvs_flash_info.led_light*16, Low_Level, DISABLE);\
tmos_stop_task(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT);tmos_stop_task(mouse_taskID, MOUSE_LED_BLINK_EVENT)
#define PWMX_LED_ALL_UPDATE() tmos_set_event(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT)
#define VBAT_CHECK_ENABLE()  tmos_start_reload_task(mouse_taskID, MOUSE_VBAT_INFO_EVENT, GET_VBAT_INFO_INTERVAL)
#define VBAT_CHECK_DISABLE() tmos_stop_task(mouse_taskID, MOUSE_VBAT_INFO_EVENT)

#define VBAT_ADC_PIN                 (GPIO_Pin_4)
#define VBAT_ADC_CHANNAL             (0)

#define KEY_C_0              (GPIO_Pin_8)       //A
#define KEY_C_1              (GPIO_Pin_9)       //A

#define KEY_R_0              (GPIO_Pin_12)       //B
#define KEY_R_1              (GPIO_Pin_13)       //
#define KEY_R_2              (GPIO_Pin_14)       //
#define KEY_R_3              (GPIO_Pin_15)       //

#define MODE_USB            0
#define MODE_2_4G           1
#define MODE_BT             2

#define  KEY_BT         (KEY_R_3)
#define  KEY_2_4G       (KEY_R_2)
#define  KEY_COM        (KEY_C_0)

#define  KEY_BT_ST         GPIOB_ReadPortPin(KEY_BT)
#define  KEY_2_4G_ST       GPIOB_ReadPortPin(KEY_2_4G)

// 基本数据
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

#define USB_DATA_LED                0xB1
#define USB_DATA_IAP                0xB2
#define USB_DATA_MANUFACTURER       0xB8

#define ENTER_SLEEP                 0x55
#define EXIT_SLEEP                  0xAA

extern uint8_t vbat_info;

extern uint8_t work_mode;
extern volatile uint8_t idel_sleep_flag; // 一档睡眠
extern volatile uint8_t deep_sleep_flag; // 二档睡眠

extern void RF_ReStart( void );
extern void access_weakup( void );
extern void access_enter_deep_sleep( void );
extern void access_enter_idel_sleep( void );
extern uint8_t trans_send_data( uint8_t *pData, uint8_t len );
void key_release(uint8_t key);
void key_press_timeout(uint8_t key);

#define CTL_MODE_BLE_1          0x31
#define CTL_MODE_BLE_2          0x32
#define CTL_MODE_BLE_3          0x33
#define CTL_MODE_BLE_4          0x34
#define CTL_MODE_BLE_5          0x35

extern void access_ctl_process( uint8_t ctl_type );
extern void access_pairing_process( uint8_t ctl_type );
#ifdef __cplusplus
}
#endif

#endif
