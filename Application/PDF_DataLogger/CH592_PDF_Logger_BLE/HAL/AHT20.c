/********************************** (C) COPYRIGHT *******************************
 * File Name          : AHT20.c
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
/* Include */
#include "HAL.h"


//#define CONFIG_AHT20_DEBUG
#ifdef CONFIG_AHT20_DEBUG
#define LOG_INFO(...)       printf(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif


/* humidity value */
float AHT20_HumidityValue = 0;

/* temperature value */
float AHT20_TemperatureValue = 0;

/**************************************************************************************************
 * @fn      HAL_AHT20_ResetCmd
 *
 * @brief   AHT20 software reset
 *
 * @param   None
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
static uint8_t HAL_AHT20_ResetCmd(void)
{
    if(HAL_I2C_WriteReg(AHT20_ADDRESS, AHT20_SOFT_RESET_REG, NULL, 0) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**************************************************************************************************
 * @fn      HAL_AHT20_CalibrateCmd
 *
 * @brief   AHT20 calibration
 *
 * @param   None
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
static uint8_t HAL_AHT20_CalibrateCmd(void)
{
    uint8_t buf[2];

    buf[0] = AHT20_CALIBRATE_REG_PARAM0;
    buf[1] = AHT20_CALIBRATE_REG_PARAM1;
    if(HAL_I2C_WriteReg(AHT20_ADDRESS, AHT20_CALIBRATE_REG, buf, 2) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**************************************************************************************************
 * @fn      HAL_AHT20_Init
 *
 * @brief   Initialize AHT20
 *
 * @param   None
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_AHT20_Init(void)
{
    uint8_t status = 0;

    HAL_I2C_Init();
    DelayMs(40);

    if (HAL_I2C_ReadCmd(AHT20_ADDRESS, &status, 1) != 0)
    {
        return 1;
    }

    if ((status & AHT20_STATUS_READY) != AHT20_STATUS_READY)  // check the status
    {
        if(HAL_AHT20_ResetCmd() != 0) //reset
        {
            return 1;
        }
        DelayMs(20);

        if(HAL_AHT20_CalibrateCmd() != 0) //calibrate
        {
            return 1;
        }
    }

    return 0;
}

/**************************************************************************************************
 * @fn      HAL_AHT20_MeasureCmd
 *
 * @brief   AHT20 starts measure
 *
 * @param   None
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_AHT20_MeasureCmd(void)
{
    uint8_t buf[2];

    buf[0] = AHT20_MEASURE_REG_PARAM0;
    buf[1] = AHT20_MEASURE_REG_PARAM1;
    if(HAL_I2C_WriteReg(AHT20_ADDRESS, AHT20_MEASURE_REG, buf, 2) != 0)
    {
        return 1;
    }

    return 0;
}

/**************************************************************************************************
 * @fn      HAL_AHT20_ReadTemperatureHumidity
 *
 * @brief   AHT20 read temperature and humidity
 *
 * @param   temperature_raw - temperature value
 *          humidity_raw - humidity value
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_AHT20_ReadTemperatureHumidity(void)
{
    uint8_t status = 0xFF;
    uint8_t buf[6];
    uint32_t temperature_raw, humidity_raw;

    while((status & 0x80) != 0)
    {
        if(HAL_I2C_ReadCmd(AHT20_ADDRESS, &status, 1) != 0)
        {
            return 1;
        }
    }

    if(HAL_I2C_ReadCmd(AHT20_ADDRESS, buf, 6) != 0)
    {
        return 1;
    }

    humidity_raw = (((uint32_t)buf[1]) << 16) |
                    (((uint32_t)buf[2]) << 8) |
                    (((uint32_t)buf[3]) << 0);          // get the humidity
    humidity_raw = humidity_raw >> 4;                   // right shift 4

    AHT20_HumidityValue = (float)humidity_raw * 100.0f / 1048576.0f;

    temperature_raw = (((uint32_t)buf[3]) << 16) |
                       (((uint32_t)buf[4]) << 8) |
                       (((uint32_t)buf[5]) << 0);       // get the temperature */
    temperature_raw = temperature_raw & 0xFFFFF;        // cut the temperature part */

    AHT20_TemperatureValue = (float)temperature_raw * 200.0f / 1048576.0f - 50.0f;

    return 0;
}


/**************************************************************************************************
 * @fn      HAL_AHT20_MeasureReadTemperatureHumidity
 *
 * @brief   AHT20 start and read temperature and humidity
 *
 * @param   temperature_raw - temperature value
 *          humidity_raw - humidity value
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_AHT20_MeasureReadTemperatureHumidity(void)
{
    uint8_t status = 0xFF;
    uint8_t buf[6];
    uint32_t temperature_raw, humidity_raw;

    buf[0] = AHT20_MEASURE_REG_PARAM0;
    buf[1] = AHT20_MEASURE_REG_PARAM1;
    if(HAL_I2C_WriteReg(AHT20_ADDRESS, AHT20_MEASURE_REG, buf, 2) != 0)
    {
        return 1;
    }

    DelayMs(75); // wait data conversion

    while((status & 0x80) != 0)
    {
        if(HAL_I2C_ReadCmd(AHT20_ADDRESS, &status, 1) != 0)
        {
            return 1;
        }
    }

    if(HAL_I2C_ReadCmd(AHT20_ADDRESS, buf, 6) != 0)
    {
        return 1;
    }

    humidity_raw = (((uint32_t)buf[1]) << 16) |
                    (((uint32_t)buf[2]) << 8) |
                    (((uint32_t)buf[3]) << 0);          // get the humidity
    humidity_raw = humidity_raw >> 4;                   // right shift 4

    AHT20_HumidityValue = (float)humidity_raw * 100.0f / 1048576.0f;

    temperature_raw = (((uint32_t)buf[3]) << 16) |
                       (((uint32_t)buf[4]) << 8) |
                       (((uint32_t)buf[5]) << 0);       // get the temperature */
    temperature_raw = temperature_raw & 0xFFFFF;        // cut the temperature part */

    AHT20_TemperatureValue = (float)temperature_raw * 200.0f / 1048576.0f - 50.0f;

    LOG_INFO("Temperature:%f | Humidity:%f\n", AHT20_TemperatureValue, AHT20_HumidityValue);

    return 0;
}
