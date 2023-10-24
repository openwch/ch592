/********************************** (C) COPYRIGHT *******************************
 * File Name          : LCD.c
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
/* Include */
#include "HAL.h"


//#define CONFIG_LCD_DEBUG
#ifdef CONFIG_LCD_DEBUG
#define LOG_INFO(...)       printf(__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

/* display numbers */
const uint8_t LCD_NUM[10]={0xFA, 0x60, 0xD6, 0xF4, 0x6C, 0xBC, 0xBE, 0xE0, 0xFE, 0xFC};

/* currently displayed value flag, 0:humidity, 1:temperature*/
uint8_t LCD_DisplayFlg = 0;

/**************************************************************************************************
 * @fn      HAL_LCD_ClearNumber
 *
 * @brief   Clear the screen of the LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_LCD_ClearScreen(void)
{
    LCD_WriteByte0(0x00);
    LCD_WriteByte1(0x00);
    LCD_WriteByte2(0x00);
    LCD_WriteByte8(0x00);
}

/**************************************************************************************************
 * @fn      HAL_LCD_ClearNumber
 *
 * @brief   Clear the numbers displayed on the LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_LCD_ClearNumber(void)
{
    LCD_WriteByte0(0x00);
    LCD_WriteByte1(0x00);
    LCD_WriteByte2(0x00);
    LCD_WriteSeg16(0x00);
}

/**************************************************************************************************
 * @fn      HAL_LCD_Init
 *
 * @brief   Initialize LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_LCD_Init(LCDDutyTypeDef duty, LCDBiasTypeDef bias)
{
    /* PA4-SEG4, PA5-SEG5 */
    GPIOA_ModeCfg(GPIO_Pin_4|GPIO_Pin_5, GPIO_ModeIN_Floating);
    /* PB0-SEG17, PB4-SEG1, PB6-SEG16, PB7-SEG0, PB22-SEG3, PB23-SEG2, PB12-COM0, PB13-COM1, PB14-COM2, PB15-COM3 */
    GPIOB_ModeCfg(GPIO_Pin_0|GPIO_Pin_4|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_12|
                  GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15|GPIO_Pin_22|GPIO_Pin_23, GPIO_ModeIN_Floating);

    R32_PIN_CONFIG2 = 0xF3D10030; // Turn off some GPIO digital inputs, PB12-15,PB22-23,PB6-PB7,PB4,PB0,PA4-PA5
    R32_LCD_CMD = 0x3003F << 8; //SEG0-5, SEG16-17
    R32_LCD_CMD |= RB_LCD_SYS_EN | RB_LCD_ON |
                   (LCD_CLK_128 << 5)  |
                   (duty << 3) |
                   (bias << 2);
}

/**************************************************************************************************
 * @fn      HAL_LCD_ShowHumidity
 *
 * @brief   Display humidity data on LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_LCD_ShowHumidity(float humidity)
{
    uint16_t temp;

    temp = (uint16_t)(humidity * 10);
    if (humidity >= 100) //show ¡®100%¡¯
    {
        LCD_WriteByte0( LCD_NUM[0] | 0x01);
        LCD_WriteByte1( LCD_NUM[0]);
        LCD_WriteByte2( LCD_NUM[0] | 0x01);
    }
    else if (humidity >= 10)
    {
        LCD_WriteByte0( LCD_NUM[temp/100]);
        LCD_WriteByte1( LCD_NUM[temp%100/10]);
        LCD_WriteByte2( LCD_NUM[temp%10] | 0x01);
    }
    else if (humidity >= 1)
    {
        LCD_WriteByte1( LCD_NUM[temp/10]);
        LCD_WriteByte2( LCD_NUM[temp%10] | 0x01);
    }
    else
    {
        LCD_WriteByte1( LCD_NUM[0]);
        LCD_WriteByte2( LCD_NUM[temp] | 0x01);
    }

    LCD_WriteSeg16( 0x08);
}

/**************************************************************************************************
 * @fn      HAL_LCD_ShowTemperature
 *
 * @brief   Display temperature data on LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_LCD_ShowTemperature(float temperature)
{
    uint16_t temp;

    temp = (uint16_t)(((temperature > 0) ? temperature : (-temperature)) * 10);
    if (temperature >= 100)
    {
        LCD_WriteByte0( LCD_NUM[temp%1000/100] | 0x01);
        LCD_WriteByte1( LCD_NUM[temp%100/10]);
        LCD_WriteByte2( LCD_NUM[temp%10] | 0x01);
    }
    else if (temperature >= 10)
    {
        LCD_WriteByte0( LCD_NUM[temp/100]);
        LCD_WriteByte1( LCD_NUM[temp%100/10]);
        LCD_WriteByte2( LCD_NUM[temp%10] | 0x01);
    }
    else if (temperature >= 1)
    {
        LCD_WriteByte1( LCD_NUM[temp/10]);
        LCD_WriteByte2( LCD_NUM[temp%10] | 0x01);
    }
    else if (temperature >= 0)
    {
        LCD_WriteByte1( LCD_NUM[0]);
        LCD_WriteByte2( LCD_NUM[temp] | 0x01);
    }
    else if (temperature > (-1))
    {
        LCD_WriteByte1( LCD_NUM[0] | 0x01);
        LCD_WriteByte2( LCD_NUM[temp] | 0x01);
    }
    else if (temperature > (-10))
    {
        LCD_WriteByte1( LCD_NUM[temp/10] | 0x01);
        LCD_WriteByte2( LCD_NUM[temp%10] | 0x01);
    }
    else
    {
        LCD_WriteByte0( LCD_NUM[temp/100]);
        LCD_WriteByte1( LCD_NUM[temp%100/10] | 0x01);
        LCD_WriteByte2( LCD_NUM[temp%10] | 0x01);
    }

    LCD_WriteSeg16( 0x04);
}

/**************************************************************************************************
 * @fn      HAL_LCD_ShowBatteryVoltage
 *
 * @brief   Display battery voltage on LCD
 *
 * @param   vol: battery voltage
 *
 * @return  None
 **************************************************************************************************/
void HAL_LCD_ShowBatteryVoltage(LCD_BatteryVoltageTypeDef vol)
{
    switch (vol)
    {
    case BAT_VOL_0:
        LCD_WriteSeg17(0x01);
        break;

    case BAT_VOL_1:
        LCD_WriteSeg17(0x03);
        break;

    case BAT_VOL_2:
        LCD_WriteSeg17(0x0B);
        break;

    case BAT_VOL_3:
        LCD_WriteSeg17(0x0F);
        break;

    default:
        break;
    }
}
