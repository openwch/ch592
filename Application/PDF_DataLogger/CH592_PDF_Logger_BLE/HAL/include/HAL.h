/********************************** (C) COPYRIGHT *******************************
 * File Name          : HAL.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2016/05/05
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#ifndef __HAL_H
#define __HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CONFIG.h"
#include "RTC.h"
#include "SLEEP.h"
#include "KEY.h"
#include "I2C.h"
#include "AHT20.h"
#include "LCD.h"
#include "BAT.h"

/* hal task Event */
#define HAL_KEY_EVENT                       0x0001
#define HAL_KEY_IDLE_TIMEOUT_EVENT          0x0002
#define HAL_AHT20_EVENT                     0x0004
#define HAL_LCD_EVENT                       0x0008
#define HAL_USB_CHECK_EVENT                 0x0010
#define HAL_DEVICE_RESET_EVENT              0x0020
#define HAL_SPI_FLASH_CLEAR_EVENT           0x0040
#define HAL_CODE_FLASH_CLEAR_EVENT          0x0080
#define HAL_REG_INIT_EVENT                  0x2000

/* Status of device */
#define DEF_DEVICE_STATUS_FACTORY   0x00
#define DEF_DEVICE_STATUS_NORMAL    0x01
#define DEF_DEVICE_STATUS_USB       0x02

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern tmosTaskID halTaskID;

/* Status of device */
extern uint8_t DeviceStatus;

extern uint8_t FlashClearFlag;

extern uint16_t FlashSectorCount;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/**
 * @brief   硬件初始化
 */
extern void HAL_Init(void);

/**
 * @brief   硬件层事务处理
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 */
extern tmosEvents HAL_ProcessEvent(tmosTaskID task_id, tmosEvents events);

/**
 * @brief   BLE 库初始化
 */
extern void CH59x_BLEInit(void);

/**
 * @brief   获取内部温感采样值，如果使用了ADC中断采样，需在此函数中暂时屏蔽中断.
 *
 * @return  内部温感采样值.
 */
extern uint16_t HAL_GetInterTempValue(void);

/**
 * @brief   内部32k校准
 */
extern void Lib_Calibration_LSI(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
