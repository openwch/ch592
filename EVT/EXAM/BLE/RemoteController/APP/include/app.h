/********************************** (C) COPYRIGHT *******************************
 * File Name          : app.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Task Events
#define APP_KEYSCAN_EVT             (1<<0)
#define APP_WEAK_UP_EVT             (1<<1)
#define APP_BATT_EVT                (1<<2)
#define APP_ENTER_IDEL_EVT          (1<<3)
#define APP_ENTER_SLEEP_EVT         (1<<4)
#define APP_SHUTDOWN_EVT            (1<<5)
#define APP_LED_UPDATE_EVT          (1<<6)
#define APP_LED_TIMEOUT_EVT         (1<<7)

#define APP_KEEP_WEAK_EVT           (1<<14)

// 10秒无按键进入空闲
#define APP_CON_IDEL_TIMEOUT     (1600*10)
// 广播状态30秒未连接进入睡眠
#define APP_ADV_SLEEP_TIMEOUT    (1600*30)
// 连接状态2分钟无按键进入睡眠
#define APP_CON_SLEEP_TIMEOUT    (1600*60*2)
// 5分钟无按键进入SHUTDOWN休眠
#define APP_SHUTDOWN_TIMEOUT     (1600*60*5)

#define APP_KEYSCAN_INT          12     // 按键检测间隔，单位0.625ms

#define APP_BATT_TIMEOUT         (1600*20)

#define APP_LED_UPDATE_INT       (1600/60) // LED灯按60Hz变化
#define APP_LED_UTIMEOUT         (1600*2) // LED灯2秒后停止
/*********************************************************************
 * MACROS
 */
#define KEY_BOOT_ID             0
#define KEY_NUM                 1

#define KEY_BOOT                GPIO_Pin_22 //用BOOT按键作为演示按键

#define KEY_DATA_BOOT           (1<<0)  //BOOT按键状态位

#define KEY_TIMEOUT             3   //长按超时，单位秒


#define VBAT_ADC_PIN            (GPIO_Pin_4)       //A
#define VBAT_ADC_CHANNAL        (0)

#define LED_PIN                 GPIO_Pin_14 //PB14作为PWM驱动led
#define LED_PWM_ID              CH_PWM10 //
/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t app_sleep_flag;
extern uint8_t app_pair_flag;
extern uint16_t hidEmuConnHandle;
/*
 * Task Initialization for the BLE Application
 */
void App_Init(void);
uint16_t App_ProcessEvent(uint8_t task_id, uint16_t events);
void Key_Scan(void);
uint8_t App_GetBattInfo(void);
void App_Update_Sleep(void);
void App_LedUpdate(void);
void App_GpioWakeupHandle(void);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
