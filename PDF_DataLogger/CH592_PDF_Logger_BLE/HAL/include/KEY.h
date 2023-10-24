/********************************** (C) COPYRIGHT *******************************
* File Name          : KEY.h
* Author             : WCH
* Version            : V1.0
* Date               : 2016/04/12
* Description        :
*******************************************************************************/



/******************************************************************************/
#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C"
{
#endif




/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
/* Key polling period */
#define HAL_KEY_POLLING_PERIOD              30 //30ms

/* Switches (keys) */
#define HAL_KEY_SW_1        0x01   // key1
#define HAL_KEY_SW_2        0x02   // key2
#define HAL_KEY_SW_3        0x04   // key3
#define HAL_KEY_SW_4        0x08   // key4

/* 按键定义 */
/* 1 - KEY */
#define KEY1_BV             BV(8)
#define KEY2_BV
#define KEY3_BV
#define KEY4_BV

#define KEY1_PU             (R32_PA_PU |= KEY1_BV)
#define KEY2_PU             ()
#define KEY3_PU             ()
#define KEY4_PU             ()

#define KEY1_DIR            (R32_PA_DIR &= ~KEY1_BV)
#define KEY2_DIR            ()
#define KEY3_DIR            ()
#define KEY4_DIR            ()

#define KEY1_IN             (ACTIVE_LOW(R32_PA_PIN & KEY1_BV))
#define KEY2_IN             ()
#define KEY3_IN             ()
#define KEY4_IN             ()

#define HAL_PUSH_BUTTON1()          ( KEY1_IN ) //添加自定义按键
#define HAL_PUSH_BUTTON2()          ( 0 )
#define HAL_PUSH_BUTTON3()          ( 0 )
#define HAL_PUSH_BUTTON4()          ( 0 )

#define WAKE_UP_KEY_RELEASE()       (R32_PA_PIN & KEY1_BV)      //睡眠唤醒引脚是否释放

/**************************************************************************************************
 * TYPEDEFS
 **************************************************************************************************/
typedef void (*halKeyCBack_t) (uint8_t keys);

/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/
extern UINT8 SHUTDOWN_FLAG;
extern UINT16 KeyIdleTime;
extern UINT16 KeyIdleTimeout;
extern UINT8 KeyPollEnFlag;


/*********************************************************************
 * FUNCTIONS
 */

/*
 * Initialize the Key Service
 */
void HAL_KeyInit( void );

/*
 * This is for internal used by hal_driver
 */
void HAL_KeyPoll( void );

/*
 * Key processing in factory mode
 */
void HAL_KeyProcessFunction_FactoryStatus(uint8_t keys);

void HAL_KeyProcessFunction_NormalStatus(uint8_t keys);

/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
