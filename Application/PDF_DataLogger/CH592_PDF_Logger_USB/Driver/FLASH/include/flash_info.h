/********************************** (C) COPYRIGHT *******************************
* File Name          : flash_info.h
* Author             : WCH
* Version            : V1.0
* Date               : 2021/10/19
* Description        :
*******************************************************************************/

/******************************************************************************/
#ifndef __FLASH_INFO_H_
#define __FLASH_INFO_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "CH59x_common.h"

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
/* Save the number of saved temperature and humidity data and PDF parameters */
#define CHECK_VALUE                     0x5A                            //Detection value of whether the information saved in flash is valid
#define DEVICEINFO_ADDR                 (0x00070000-FLASH_ROM_MAX_SIZE) //Starting address of information saving in flash (DataFlash)
#define DEVICEINFO_FLASH_SIZE           (8*1024)                        //Size used to save information in flash(8k)
#define DEVICEINFO_LEN                  44                              //Single information size
#define DEVICEINFO_MAX                  (DEVICEINFO_FLASH_SIZE/DEVICEINFO_LEN)                          //Number of information that can be saved in flash

/* FLASH to save temperature and humidity data */
#define MEASURENT_DATA_ADDR             (268*1024)                      //Starting address of measurement data saving in flash (CodeFlash)
#define MEASURENT_DATA_FLASH_SIZE       (180*1024)                      //Size used to save information in flash(100k)
#define TEMP_DATA_LEN                   2                               //Temperature data size
#define HUMI_DATA_LEN                   2                               //Humidity data size
#define MEASURENT_DATA_MAX              (MEASURENT_DATA_FLASH_SIZE/(TEMP_DATA_LEN+HUMI_DATA_LEN))       //Number of measurement data that can be saved in flash

/* Default device information parameters */
#define CELSIUS_UNIT                    0x00                            //Celsius
#define FAHRENHEIT_UNIT                 0x01                            //Fahrenheit
#define DEFAULT_MEASURE_INTERVAL        6                               //Measurement interval
#define DEFAULT_TEMP_UPPER_LIMIT        (float)(35.0)                   //Temperature upper limit
#define DEFAULT_TEMP_LOWER_LIMIT        (float)(-10.0)                  //Temperature lower limit
#define DEFAULT_HUMI_UPPER_LIMIT        (float)(90.0)                   //Humidity upper limit
#define DEFAULT_HUMI_LOWER_LIMIT        (float)(30.0)                   //Humidity lower limit
#define DEFAULT_START_YAER              2023                            //Year
#define DEFAULT_START_MONTH             9                               //Month
#define DEFAULT_START_DAY               20                               //Day
#define DEFAULT_START_HOUR              0                               //Hour
#define DEFAULT_START_MINUTE            0                               //Minute
#define DEFAULT_START_SECOND            0                               //Second

#ifndef SUCCESS
#define SUCCESS                         0x00
#endif
#ifndef FAILURE
#define FAILURE                         0x01
#endif

/**************************************************************************************************
 *                                               TYPEDEFS
 **************************************************************************************************/
/* define PDF parameters structure */
typedef struct PDF_PARAM
{
    uint16_t MeasureInterval;        //Measurement interval, units of 1min
    uint8_t TempUnit;               //Temperature unit, 0:°„C, 1:°„F
    float MaxTempAlarm;             //Alarm temperature upper limit
    float MinTempAlarm;             //Alarm temperature lower limit
    float MaxHumiAlarm;             //Alarm humidity upper limit
    float MinHumiAlarm;             //Alarm humidity lower limit
} PDF_PARAM_t;

/* define PDF parameters structure */
typedef struct START_TIME
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
} START_TIME_t;

/* define device information structure */
typedef struct DEVICE_INFO
{
    uint8_t CheckFlag;              //Message valid flag
    uint16_t InfoNum;               //Number of information stored in flash. Erase the flash once when it is full.
    uint8_t StatusFlag;             //0-factory setting state(enable few functions), 1-normal operation state
    PDF_PARAM_t PdfParam;           //PDF related parameters
    uint32_t MeasureNum;            //Number of saved measurement data
    START_TIME_t StartTime;         //System startup time
    uint8_t Reserve[3];             //Reserve
    uint8_t Checksum;               //Checksum of the information
} DEVICE_INFO_t;

/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/
extern DEVICE_INFO_t DeviceInfo;       //Device Information

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Save device information in flash
 */
uint8_t HAL_SaveDeviceInfo(void);

/*
 * Read the device information in the flash
 */
uint8_t HAL_ReadDeviceInfo(void);

/*
 * Calculate buffer checksum
 */
uint8_t HAL_FlashChecksumCalculate(uint8_t *p_buf, uint16_t len);

/*
 * Check buffer checksum
 */
uint8_t HAL_FlashChecksumCheck(uint8_t *p_buf, uint16_t len);


/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_INFO_H_ */
