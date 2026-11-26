/********************************** (C) COPYRIGHT *******************************
 * File Name          : adc_cfg.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/11/17
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __ADC_CFG_H__
#define __ADC_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

extern signed short     RoughCalib_Value;
extern uint16_t  adcBuff[ 100 ];
extern volatile  uint8_t SCAN_end;

/**
 * @brief   ADC通道GPIO配置
 *
 * @param   ch      - ADC通道
 */
void ADC_AinGpioCfg( uint8_t ch );

/**
 * @brief   ADC扫描
 *
 * @param   none.
 */
void ADC_Scan( void );

/**
 * @brief   ADC等待转换完成, 在ram中等待
 *
 * @param   none.
 */
void ADC_StartandWait( void );

#ifdef __cplusplus
}
#endif

#endif // __ADC_CFG_H__
