/********************************** (C) COPYRIGHT *******************************
 * File Name          : LCD.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/09/01
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/

#ifndef __LCD_H_
#define __LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CH59x_lcd.h"

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
#define LCD_WriteByte0( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xffffff00) | ((UINT32)(d)))          /* 填充SEG0,SEG1驱动数值 */
#define LCD_WriteByte1( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xffff00ff) | ((UINT32)(d)<<8))       /* 填充SEG2,SEG3驱动数值 */
#define LCD_WriteByte2( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xff00ffff) | ((UINT32)(d)<<16))      /* 填充SEG4,SEG5驱动数值 */
#define LCD_WriteByte3( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0x00ffffff) | ((UINT32)(d)<<24))      /* 填充SEG6,SEG7驱动数值 */
#define LCD_WriteByte4( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xffffff00) | ((UINT32)(d)))          /* 填充SEG8,SEG9驱动数值 */
#define LCD_WriteByte5( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xffff00ff) | ((UINT32)(d)<<8))       /* 填充SEG10,SEG11驱动数值 */
#define LCD_WriteByte6( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xff00ffff) | ((UINT32)(d)<<16))      /* 填充SEG12,SEG13驱动数值 */
#define LCD_WriteByte7( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0x00ffffff) | ((UINT32)(d)<<24))      /* 填充SEG14,SEG15驱动数值 */
#define LCD_WriteByte8( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffffff00) | ((UINT32)(d)))          /* 填充SEG16,SEG17驱动数值 */
#define LCD_WriteByte9( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffff00ff) | ((UINT32)(d)<<8))       /* 填充SEG18,SEG19驱动数值 */

#define LCD_WriteSeg16( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xfffffff0) | ((UINT32)(d)))          /* 填充SEG16驱动数值 */
#define LCD_WriteSeg17( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffffff0f) | ((UINT32)(d)<<4))       /* 填充SEG17驱动数值 */

/**
  * @brief  LCD display battery voltage
  */
typedef enum
{
    BAT_VOL_0 = 0,          //no power
    BAT_VOL_1,              //1 bar power
    BAT_VOL_2,              //2 bar power
    BAT_VOL_3               //3 bar power
}LCD_BatteryVoltageTypeDef;

/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/
/* currently displayed value flag, 0:humidity, 1:temperature*/
extern uint8_t LCD_DisplayFlg;

/*********************************************************************
 * FUNCTIONS
 */

/**
 * @brief   Clear the screen of the LCD
 */
void HAL_LCD_ClearScreen(void);

/**
 * @brief   Clear the numbers displayed on the LCD
 */
void HAL_LCD_ClearNumber(void);

/**
 * @brief   Initialize LCD
 */
void HAL_LCD_Init(LCDDutyTypeDef duty, LCDBiasTypeDef bias);

/**
 * @brief   Display humidity data on LCD
 */
void HAL_LCD_ShowHumidity(float humidity);

/**
 * @brief   Display temperature data on LCD
 */
void HAL_LCD_ShowTemperature(float temperature);

/**
 * @brief   Display battery voltage on LCD
 */
void HAL_LCD_ShowBatteryVoltage(LCD_BatteryVoltageTypeDef vol);


/********************************************************************************
********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H_ */
