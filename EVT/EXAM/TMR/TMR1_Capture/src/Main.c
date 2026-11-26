/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : TMR1捕捉例程，PA10边沿捕捉+DMA存储，PA10为LSE晶振引脚需关闭LSE
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

__attribute__((aligned(4))) uint32_t CapBuf[102]; // 注意前两个数据为无效数据

volatile uint8_t capFlag = 0;

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
    PRINT("TMR1 capture on PA10, DMA single mode\n");

    uint8_t i;

    PWR_UnitModCfg(DISABLE, UNIT_SYS_LSE); // 注意此引脚是LSE晶振引脚，要保证关闭才能使用其他功能
    GPIOA_ResetBits(GPIO_Pin_10);          // 配置捕捉脚 PA10
    GPIOA_ModeCfg(GPIO_Pin_10, GPIO_ModeIN_PU);

    TMR1_CapInit(Edge_To_Edge);
    TMR1_CAPTimeoutCfg(33554432); // max capture timeout per CH59x_timer.h (33554432)
    TMR1_DMACfg(ENABLE, (uint16_t)(uint32_t)&CapBuf[0], (uint16_t)(uint32_t)&CapBuf[102], Mode_Single);
    TMR1_ITCfg(ENABLE, TMR1_2_IT_DMA_END); // 开启DMA完成中断
    PFIC_EnableIRQ(TMR1_IRQn);

    while(capFlag == 0);
    capFlag = 0;
    PRINT("capture data:\n");
    for(i = 2; i < 102; i++)   // 注意前两个数据为无效数据
    {
        PRINT("%08ld ", CapBuf[i] & 0x1ffffff); // 26bit, 最高位表示 高电平还是低电平
    }
    PRINT("\n");

    while(1);
}

/*********************************************************************
 * @fn      TMR1_IRQHandler
 *
 * @brief   TMR1中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler(void) // TMR1 捕捉DMA完成中断
{
    if(TMR1_GetITFlag(TMR1_2_IT_DMA_END))
    {
        TMR1_ITCfg(DISABLE, TMR1_2_IT_DMA_END); // 使用单次DMA功能+中断，注意完成后关闭此中断使能，否则会一直上报中断。
        TMR1_ClearITFlag(TMR1_2_IT_DMA_END);    // 清除中断标志
        capFlag = 1;
    }
}
