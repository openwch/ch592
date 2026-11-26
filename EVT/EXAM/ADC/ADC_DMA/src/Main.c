/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : ADC单通道DMA采样例程，PA4(AIN0)自动连续采样40点
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

uint16_t adcBuff[40];

volatile uint8_t DMA_end = 0;

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
    PRINT("ADC single channel PA4, DMA 40 points\n");

    uint8_t i;

    /* DMA单通道采样：选择adc通道0做采样，对应 PA4引脚 */
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    ADC_ChannelCfg(0);
    ADC_AutoConverCycle(192); // 采样周期为 (256-192)*16个系统时钟，周期太小可能会导致中断关闭DMA不及时
    DMA_end = 0;
    /* ADC_DMA_END is the exclusive buffer end (per CH592 RM): DMA fills adcBuff[0..39].
       Keep &adcBuff[40] as the open upper bound; do not use &adcBuff[39] here. */
    ADC_DMACfg(ENABLE, (uint32_t)&adcBuff[0], (uint32_t)&adcBuff[40], ADC_Mode_Single);
    PFIC_EnableIRQ(ADC_IRQn);
    ADC_StartAutoDMA();
    while(!DMA_end);
    ADC_DMACfg(DISABLE, 0, 0, 0);

    for(i = 0; i < 40; i++)
    {
        PRINT("%d \n", adcBuff[i]);
    }

    while(1);
}

/*********************************************************************
 * @fn      ADC_IRQHandler
 *
 * @brief   ADC中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void ADC_IRQHandler(void) // adc中断服务程序
{
    if(ADC_GetDMAStatus())
    {
        // 停止DMA，如果关闭不及时，会导致自动启动第二次DMA，覆写之前采样的数据
        ADC_StopAutoDMA();
        R16_ADC_DMA_BEG = ((uint32_t)adcBuff) & 0xffff;
        ADC_ClearDMAFlag();
        DMA_end = 1;
    }
}
