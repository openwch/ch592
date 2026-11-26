/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : ADC TouchKey采样例程，选择adc通道2，对应PA12
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

uint16_t adcBuff[20];

/*********************************************************************
 * @fn      DebugInit
 *
 * @brief   调试初始化
 *
 * @return  none
 */
void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main(void)
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* 配置串口调试 */
    DebugInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);
    PRINT("ADC TouchKey sampling on PA12\n");

    uint8_t i;

    /* TouchKey采样：选择adc通道 2 做采样，对应 PA12 */
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating);
    TouchKey_ChSampInit();
    ADC_ChannelCfg(2);

    for(i = 0; i < 20; i++)
    {
        adcBuff[i] = TouchKey_ExcutSingleConver(0x10, 0); // 连续采样20次
    }
    for(i = 0; i < 20; i++)
    {
        PRINT("%d \n", adcBuff[i]);
    }

    while(1);
}
