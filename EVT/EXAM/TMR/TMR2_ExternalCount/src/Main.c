/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : TMR2外部脉冲计数例程，PB11重映射输入，计数满1000中断
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
    PRINT("TMR2 external count on PB11, overflow 1000\n");

    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PD);
    GPIOPinRemap(ENABLE, RB_PIN_TMR2);

    TMR2_EXTSingleCounterInit(FallEdge_To_FallEdge);
    TMR2_CountOverflowCfg(1000); // 设置计数上限1000

    /* 开启计数溢出中断，计满1000个周期进入中断 */
    TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR2_IRQn);
    TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);

    do
    {
        /* 1s打印一次当前计数值，如果送入脉冲频率较高，可能很快计数溢出，需要按实际情况修改 */
        mDelaymS(1000);
        PRINT("=%ld \n", TMR2_GetCurrentCount());
    } while(1);
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
    if(TMR2_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
        /* 计数器计满，硬件自动清零，重新开始计数 */
        /* 用户可自行添加需要的处理 */
    }
}
