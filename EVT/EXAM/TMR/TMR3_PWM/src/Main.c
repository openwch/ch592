/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : TMR3 PWM输出例程，PB22输出100us周期50%占空比
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

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
    PRINT("TMR3 PWM on PB22: 100us period, 50%% duty\n");

    GPIOB_ResetBits(GPIO_Pin_22); // 配置PWM口 PB22
    GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeOut_PP_5mA);
    TMR3_PWMInit(High_Level, PWM_Times_1);
    TMR3_PWMCycleCfg(60 * 100); // 周期 100us  最大67108864
    TMR3_PWMActDataWidth(3000); // 占空比 50%, 修改占空比必须暂时关闭定时器
    TMR3_PWMEnable();
    TMR3_Enable();

    while(1);
}
