/********************************** (C) COPYRIGHT *******************************
 * File Name          : BAT.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/01/27
 * Description        :
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "HAL.h"

//#define CONFIG_BAT_DEBUG
#ifdef CONFIG_BAT_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
static signed short RoughCalib_Value = 0;    // ADC coarse adjustment deviation value
//uint8 BatVolStartFlg = 0; //电池电压获取开始

/**************************************************************************************************
 *                                          FUNCTIONS
 **************************************************************************************************/

/**************************************************************************************************
 * @fn      HAL_BatVolInit
 *
 * @brief   Initilize BAT Service
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_BatVolInit(void)
{
    ADC_InterBATSampInit();
    RoughCalib_Value = ADC_DataCalib_Rough();
    LOG_INFO("RoughCalib_Value = %d\n", RoughCalib_Value);
}

/**************************************************************************************************
 * @fn      ADC_GetAverage
 *
 * @brief   Get the average value of ADC
 *
 * @param   times - number
 *
 * @return  Average value of ADC
 **************************************************************************************************/
uint16_t ADC_GetAverage(uint8_t times)
{
    uint8_t i;
    uint16_t ADCVal = 0;
    uint16_t ADCSum = 0;

    ADCVal = ADC_ExcutSingleConver(); //First converted value is discarded
    for(i=0; i<times; ++i)
    {
        ADCVal = ADC_ExcutSingleConver();

        if((RoughCalib_Value >= 0) || (~(RoughCalib_Value-1) < ADCVal))
        {
            ADCVal += RoughCalib_Value;
        }

        ADCSum += ADCVal;
    }

    return (ADCSum/times);
}

/**************************************************************************************************
 * @fn      HAL_BatLowVolCheck
 *
 * @brief   Check whether the battery voltage is too low
 *
 * @param   None
 *
 * @return  TRUE or FALSE
 **************************************************************************************************/
uint8_t HAL_BatLowVolCheck(void)
{
    uint16_t adc;

    adc = ADC_GetAverage(8); //ADC采集8次
//    LOG_INFO("BAT_ADC = %d\n", adc);

    if(adc > BATT_ADC_LEVEL_2V3)
    {
        LOG_INFO("BAT voltage more than 2.3V\n");
        return FALSE;
    }
    else
    {
        LOG_INFO("BAT voltage less than 2.3V\n");
        return TRUE;
    }
}



/******************************** endfile @ bat ******************************/
