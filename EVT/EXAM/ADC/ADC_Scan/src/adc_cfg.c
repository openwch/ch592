/********************************** (C) COPYRIGHT *******************************
 * File Name          : adc_cfg.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"
#include "adc_cfg.h"


/* CH592 的外部 ADC 通道共 12 个: AIN0~AIN5、AIN8~AIN13（无 AIN6/AIN7），
   扫描队列最多容纳 12 个通道: 通道0(R8_ADC_CHANNEL) + 通道1~11(SCAN_CFG1/SCAN_CFG2) */
#define   ADC_SCAN_CH_NUM    12

signed short     RoughCalib_Value = 0;  // ADC粗调偏差值
uint16_t  adcBuff[ 100 ];
volatile  uint8_t SCAN_end = 0;
static volatile uint8_t SCAN_timeout = 0;

uint8_t scan_ch[ 12 ] = {
    CH_EXTIN_0, CH_EXTIN_1, CH_EXTIN_2, CH_EXTIN_3,
    CH_EXTIN_4, CH_EXTIN_5, CH_EXTIN_8, CH_EXTIN_9, 
    CH_EXTIN_10, CH_EXTIN_11,CH_EXTIN_12,CH_EXTIN_13 };

/* ---- ADC 通道 GPIO 引脚映射 ---- */
/** AIN0~AIN5 对应 GPIOA 的引脚（6 个通道） */
static const uint32_t ain_pin_a0[] = {
    GPIO_Pin_4,   // AIN0
    GPIO_Pin_5,   // AIN1
    GPIO_Pin_12,  // AIN2
    GPIO_Pin_13,  // AIN3
    GPIO_Pin_14,  // AIN4
    GPIO_Pin_15,  // AIN5
};

/** AIN8~AIN9 对应 GPIOB 的引脚（2 个通道） */
static const uint32_t ain_pin_b[] = {
    GPIO_Pin_0,  // AIN8
    GPIO_Pin_6,  // AIN9
};

/** AIN10~AIN13 对应 GPIOA 的引脚（4 个通道） */
static const uint32_t ain_pin_a1[] = {
    GPIO_Pin_6,   // AIN10
    GPIO_Pin_7,   // AIN11
    GPIO_Pin_8,   // AIN12
    GPIO_Pin_9,   // AIN13
};

/*********************************************************************
 * @fn      ADC_StartandWait
 *
 * @brief   ADC等待转换完成, 需在ram中等待
 *
 * @return  none
 */
__HIGH_CODE
void ADC_StartandWait( void )
{
    uint32_t timeout = 0;

    R8_ADC_CONVERT = RB_ADC_START;  /* 启动扫描 */
    while ( !SCAN_end )
    {
        if ( ++timeout > 3000000 )
        {
            SCAN_timeout = 1;       /* 扫描异常时超时退出, 避免死等 */
            break;
        }
    }
    SCAN_end = 0;
}

/*********************************************************************
 * @fn      ADC_AinGpioCfg
 *
 * @brief   ADC通道GPIO配置
 *
 * @return  none
 */
void ADC_AinGpioCfg( uint8_t ch )
{
    if ( ch <= CH_EXTIN_5 )
    {
        GPIOA_ModeCfg( ain_pin_a0[ ch ], GPIO_ModeIN_Floating );
    }
    else if ( ( ch == CH_EXTIN_8 ) || ( ch == CH_EXTIN_9 ) )
    {
        GPIOB_ModeCfg( ain_pin_b[ ch - CH_EXTIN_8 ], GPIO_ModeIN_Floating );
    }
    else if ( ( ch >= CH_EXTIN_10 ) && ( ch <= CH_EXTIN_13 ) )
    {
        GPIOA_ModeCfg( ain_pin_a1[ ch - CH_EXTIN_10 ], GPIO_ModeIN_Floating );
    }
    else
    {
        // CH592 无 AIN6/AIN7 通道
        PRINT( "err\n" );
    }
}

/*********************************************************************
 * @fn      ADC_Scan
 *
 * @brief   ADC扫描
 *
 * @return  none
 */
void ADC_Scan( void )
{
    memset( adcBuff, 0, sizeof( adcBuff ) );

    /* 配置本次扫描用到的 12 个 AIN 引脚为浮空输入 */
    for ( int i = 0; i < ADC_SCAN_CH_NUM; i++ )
    {
        ADC_AinGpioCfg( scan_ch[ i ] );
    }

    ADC_ExtSingleChSampInit( SampleFreq_4, ADC_PGA_0 );

    RoughCalib_Value = ADC_DataCalib_Rough();
    PRINT( "RoughCalib_Value =%d \n", RoughCalib_Value );
    ADC_ChannelCfg( scan_ch[ 0 ] );

    /* 打包通道 1~8 到 SCAN_CFG1，每通道 4 位 */
    uint32_t cfg1     = 0;
    for ( int i = 0; i < 8; i++ )
    {
        cfg1 |= ( (uint32_t)( scan_ch[ i + 1 ] ) << ( i * 4 ) );
    }
    R32_ADC_SCAN_CFG1 = cfg1;

    /* 打包通道 9~11 到 SCAN_CFG2，连同扫描控制位一次写入
       （分多次 |= 会出现 SCAN_NUM 已非 0 而 SCAN_SEL 仍为 0 的中间态，即短暂切到 TKEY 扫描） */
    uint32_t cfg2 = 0;
    for ( int i = 0; i < 3; i++ )
    {
        cfg2 |= ( (uint32_t)( scan_ch[ i + 9 ] ) << ( i * 4 ) );
    }
    cfg2 |= ( ( ( ADC_SCAN_CH_NUM - 1 ) & 0x0F ) << 16 ); /* scan_num = 12-1=11 ADC_SCAN_CH_NUM - 1*/
    cfg2 |= RB_ADC_SCAN_SEL;                              /* 选择 ADC扫描(非 TKEY) */
    cfg2 |= RB_ADC_IE_SCAN_END;                           /* 扫描结束中断使能*/
    R32_ADC_SCAN_CFG2 = cfg2;

    R32_ADC_DMA_BEG  = (uint32_t)&adcBuff[ 0 ] & 0x7FFE;
    R32_ADC_DMA_END  = (uint32_t)&adcBuff[ ADC_SCAN_CH_NUM + 1 ] & 0x7FFE;
    R8_ADC_CTRL_DMA |= RB_ADC_IE_DMA_END|RB_ADC_DMA_ENABLE;
    PFIC_EnableIRQ( ADC_IRQn );
    SCAN_end     = 0;
    SCAN_timeout = 0;
    ADC_StartandWait();

    /* 扫描未在预期时间内结束时打印现场，便于定位（正常时不打印） */
    if ( SCAN_timeout )
    {
        PRINT( "SCAN timeout! CONVERT=%02X CFG1=%08X CFG2=%08X CTRL_DMA=%02X DMA_IF=%02X DMA_NOW=%04X CH=%02X\n",
               R8_ADC_CONVERT, (unsigned int)R32_ADC_SCAN_CFG1, (unsigned int)R32_ADC_SCAN_CFG2,
               R8_ADC_CTRL_DMA, R8_ADC_DMA_IF, R16_ADC_DMA_NOW, R8_ADC_CHANNEL );
    }

    PFIC_DisableIRQ( ADC_IRQn );
    ADC_DMACfg( DISABLE, 0, 0, 0 );

    /* 关闭扫描模式 */
    R32_ADC_SCAN_CFG2 = 0;

    for ( int i = 0; i < ADC_SCAN_CH_NUM; i++ )
    {
        PRINT( "SCAN_CH%d: %d \n", i, adcBuff[ i ] + RoughCalib_Value );
    }
}
