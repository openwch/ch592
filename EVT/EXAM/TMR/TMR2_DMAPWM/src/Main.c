/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : TMR2 DMA PWM例程，PB11重映射输出，DMA循环渐变占空比
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

__attribute__((aligned(4))) uint32_t PwmBuf[100];

/* DMA end flag: set in ISR, printed in main loop (never print inside ISR) */
volatile uint8_t dmaEndFlag = 0;

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
    PRINT("TMR2 DMA PWM on PB11, loop triangle duty\n");

    uint8_t i;

    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeOut_PP_5mA);
    GPIOPinRemap(ENABLE, RB_PIN_TMR2);

    TMR2_PWMCycleCfg(60 * 2000); // 周期 2000us  主频是60Mhz 每秒震荡60M次 震荡60次为1微秒
    for(i = 0; i < 50; i++)
    {
        PwmBuf[i] = 2400 * i;
    }
    for(i = 50; i < 100; i++)
    {
        PwmBuf[i] = 2400 * (100 - i);
    }
    TMR2_PWMInit(Low_Level, PWM_Times_16);
    TMR2_DMACfg(ENABLE, (uint32_t)&PwmBuf[0], (uint32_t)&PwmBuf[100], Mode_LOOP);
    TMR2_PWMEnable();
    TMR2_Enable();
    /* 开启DMA完成中断 */
    TMR2_ClearITFlag(TMR1_2_IT_DMA_END);
    TMR2_ITCfg(ENABLE, TMR1_2_IT_DMA_END);
    PFIC_EnableIRQ(TMR2_IRQn);

    while(1)
    {
        if(dmaEndFlag)
        {
            dmaEndFlag = 0;
            PRINT("DMA end\n");
        }
    }
}

/*********************************************************************
 * @fn      TMR2_IRQHandler
 *
 * @brief   TMR2中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void)
{
    if(TMR2_GetITFlag(TMR1_2_IT_DMA_END))
    {
        TMR2_ClearITFlag(TMR1_2_IT_DMA_END);
        dmaEndFlag = 1; // only set flag here, do not print in ISR
        /* DMA 结束 */
        /* 用户可自行添加需要的处理 */
    }
}
