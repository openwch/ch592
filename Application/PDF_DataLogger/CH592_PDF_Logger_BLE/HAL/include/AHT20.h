/********************************** (C) COPYRIGHT *******************************
 * File Name          : AHT20.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/09/01
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/

#ifndef __AHT20_H_
#define __AHT20_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
/* device address */
#define AHT20_ADDRESS               0x38        //I2C device address

/* device status */
#define AHT20_STATUS_READY          0x18        //I2C is ready

/* list of command registers */
#define AHT20_SOFT_RESET_REG        0xBA        //soft reset register
#define AHT20_CALIBRATE_REG         0xBE        //soft calibration register
#define AHT20_CALIBRATE_REG_PARAM0  0x08
#define AHT20_CALIBRATE_REG_PARAM1  0x00
#define AHT20_MEASURE_REG           0xAC        //soft start measure register
#define AHT20_MEASURE_REG_PARAM0    0x33
#define AHT20_MEASURE_REG_PARAM1    0x00


/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/
/* humidity value */
extern float AHT20_HumidityValue;

/* temperature value */
extern float AHT20_TemperatureValue;


/*********************************************************************
 * FUNCTIONS
 */

/**
 * @brief   Initialize AHT20
 */
uint8_t HAL_AHT20_Init(void);

/**
 * @brief   AHT20 starts measure
 */
uint8_t HAL_AHT20_MeasureCmd(void);

/**
 * @brief   AHT20 read temperature and humidity
 */
uint8_t HAL_AHT20_ReadTemperatureHumidity(void);

/**
 * @brief   AHT20 start and read temperature and humidity
 */
uint8_t HAL_AHT20_MeasureReadTemperatureHumidity(void);


/********************************************************************************
********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __AHT20_H_ */
