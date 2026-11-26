/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/03/09
 * Description        : adc通道扫描功能例程，对12个外部输入通道(AIN0~AIN5、AIN8~AIN13)依次扫描采样，DMA自动存储各通道结果，扫描结束在中断中置完成标志。
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"
#include "adc_cfg.h"

/*********************************************************************
 * @fn      DebugInit
 *
 * @brief   调试初始化
 *
 * @return  none
 */
void DebugInit( void )
{
    GPIOB_SetBits( bTXD0 );
    GPIOB_ModeCfg( bRXD0, GPIO_ModeIN_PU );
    GPIOB_ModeCfg( bTXD0, GPIO_ModeOut_PP_5mA );
    UART0_DefInit();
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main()
{
    SetSysClock( CLK_SOURCE_PLL_60MHz );

    /* 配置串口调试 */
    DebugInit();
    PRINT( "Time:%s; Start @ChipID=%02X\n", __TIMESTAMP__, R8_CHIP_ID );

    PRINT( "\nADC 12ch Scan...\n" );
    ADC_Scan();

    while ( 1 );
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
void ADC_IRQHandler( void )  // adc中断服务程序：扫描结束
{

    if ( R32_ADC_SCAN_CFG2 & RB_ADC_IF_SCAN_END )
    {
        R32_ADC_SCAN_CFG2 |= RB_ADC_IF_SCAN_END;
        R16_ADC_DMA_BEG = (uint32_t)adcBuff & 0xffff;
        SCAN_end           = 1;
    }
}
