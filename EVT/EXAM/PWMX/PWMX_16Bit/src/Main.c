/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : PWM4-PWM9 16位数据宽度输出例程，含周期切换中断演示(默认关闭)
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

volatile uint16_t nextPWMCyc;

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
    PRINT("PWMX 16bit: PWM4-9 output\n");

    /* 配置GPIO */
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeOut_PP_5mA); // PA12 - PWM4
    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA); // PA13 - PWM5
    GPIOB_ModeCfg(GPIO_Pin_0, GPIO_ModeOut_PP_5mA);  // PB0 - PWM6
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeOut_PP_5mA);  // PB4 - PWM7
    GPIOB_ModeCfg(GPIO_Pin_6, GPIO_ModeOut_PP_5mA);  // PB6 - PWM8
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);  // PB7 - PWM9

    PWMX_CLKCfg(4);                                        // cycle = 4/Fsys
    PWMX_16bit_CycleCfg(60000 - 1);                        // 16位数据宽度Ncyc=RB_PWM_CYC_VALUE+1
    PWMX_16bit_ACTOUT(CH_PWM4, 30000, Low_Level, ENABLE);  // 50%占空比
    PWMX_16bit_ACTOUT(CH_PWM5, 15000, Low_Level, ENABLE);  // 25%占空比
    PWMX_16bit_ACTOUT(CH_PWM6, 45000, Low_Level, ENABLE);  // 75%占空比
    PWMX_16bit_ACTOUT(CH_PWM7, 30000, High_Level, ENABLE); // 50%占空比
    PWMX_16bit_ACTOUT(CH_PWM8, 15000, High_Level, ENABLE); // 25%占空比
    PWMX_16bit_ACTOUT(CH_PWM9, 45000, High_Level, ENABLE); // 75%占空比

#if 0
    // 切换周期演示，在进行周期切换时需要等到上一个周期完成后再切，以PWM4为例
    nextPWMCyc = 30000 - 1;
    PWM_INTCfg(ENABLE, RB_PWM_IE_CYC);
    PFIC_EnableIRQ(PWMX_IRQn);
#endif

    while(1);
}

/*********************************************************************
 * @fn      PWMX_IRQHandler
 *
 * @brief   PWMX中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void PWMX_IRQHandler(void)
{
    if(R8_PWM_INT_CTRL & RB_PWM_IF_CYC)
    {
        R8_PWM_INT_CTRL = RB_PWM_IF_CYC;
        PWMX_16bit_ACTOUT(CH_PWM4, nextPWMCyc / 4, High_Level, ENABLE); // 切换周期时需要先改变占空比
        PWMX_16bit_CycleCfg(nextPWMCyc);
    }
    PFIC_DisableIRQ(PWMX_IRQn);
}
