/********************************** (C) COPYRIGHT *******************************
* File Name          : flash_info.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/10/19
* Description        :
*******************************************************************************/

/******************************************************************************/
#include "flash_info.h"
#include "spi_flash.h"

//#define CONFIG_FLASH_INFO_DEBUG
#ifdef CONFIG_FLASH_INFO_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
DEVICE_INFO_t DeviceInfo;       //Device Information

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/**************************************************************************************************
 * @fn      HAL_SaveBondInfo
 *
 * @brief   Save bond information in flash
 *
 * @param   None
 *
 * @return  SUCCESS or FAILURE
 **************************************************************************************************/
uint8_t HAL_SaveDeviceInfo(void)
{
    uint32_t StartAddr;

    if (DeviceInfo.InfoNum == DEVICEINFO_MAX) //flashÒÑ¾­´æ´¢Âú
    {
        if (EEPROM_ERASE(DEVICEINFO_ADDR, DEVICEINFO_FLASH_SIZE) != SUCCESS)
        {
            LOG_INFO("ERROR1\n");
            return FAILURE;
        }
        DeviceInfo.InfoNum = 0;
    }

    StartAddr = DEVICEINFO_ADDR + DEVICEINFO_LEN * DeviceInfo.InfoNum;
    ++DeviceInfo.InfoNum; //Add in advance and need to save it in flash
    DeviceInfo.Checksum = HAL_FlashChecksumCalculate((uint8_t*)&DeviceInfo, DEVICEINFO_LEN-1);
    if (EEPROM_WRITE(StartAddr, &DeviceInfo, DEVICEINFO_LEN) != SUCCESS) //Ð´flash
    {
        --DeviceInfo.InfoNum;
        LOG_INFO("ERROR2\n");
        return FAILURE;
    }

    return SUCCESS;
}

/**************************************************************************************************
 * @fn      HAL_ReadBondInfo
 *
 * @brief   Read the bond information in the flash
 *
 * @param   None
 *
 * @return  SUCCESS or FAILURE
 **************************************************************************************************/
uint8_t HAL_ReadDeviceInfo(void)
{
    uint32_t StartAddr;
    uint16_t BufNum = 1;

    do
    {
        StartAddr = DEVICEINFO_ADDR + BufNum * DEVICEINFO_LEN;
        EEPROM_READ(StartAddr, &DeviceInfo, 1);
        ++BufNum;
    } while ((DeviceInfo.CheckFlag != 0xFF) && (BufNum <= DEVICEINFO_MAX)); //Poll flash until find where the device information is stored

    if (BufNum > (DEVICEINFO_MAX + 1))
    {
        LOG_INFO("ERROR3\n");
        return FAILURE;
    }

    StartAddr = DEVICEINFO_ADDR + (BufNum - 2) * DEVICEINFO_LEN;
    EEPROM_READ(StartAddr, &DeviceInfo, DEVICEINFO_LEN); //Read device information in flash
    if ((DeviceInfo.CheckFlag != CHECK_VALUE) || (HAL_FlashChecksumCheck((uint8_t*)&DeviceInfo, DEVICEINFO_LEN-1))) //If device information has not been saved
    {
        memset((uint8_t*)&DeviceInfo, 0, DEVICEINFO_LEN);
        DeviceInfo.CheckFlag = CHECK_VALUE;
        DeviceInfo.InfoNum = DEVICEINFO_MAX;
        DeviceInfo.PdfParam.MeasureInterval = DEFAULT_MEASURE_INTERVAL; //6min
        DeviceInfo.PdfParam.TempUnit = CELSIUS_UNIT;
        DeviceInfo.PdfParam.MaxTempAlarm = DEFAULT_TEMP_UPPER_LIMIT;
        DeviceInfo.PdfParam.MinTempAlarm = DEFAULT_TEMP_LOWER_LIMIT;
        DeviceInfo.PdfParam.MaxHumiAlarm = DEFAULT_HUMI_UPPER_LIMIT;
        DeviceInfo.PdfParam.MinHumiAlarm = DEFAULT_HUMI_LOWER_LIMIT;
        DeviceInfo.StartTime.Year = DEFAULT_START_YAER;
        DeviceInfo.StartTime.Month = DEFAULT_START_MONTH;
        DeviceInfo.StartTime.Day = DEFAULT_START_DAY;
//        DeviceInfo.StartTime.Hour = DEFAULT_START_HOUR;
//        DeviceInfo.StartTime.Minute = DEFAULT_START_MINUTE;
//        DeviceInfo.StartTime.Second = DEFAULT_START_SECOND;

        if (HAL_SaveDeviceInfo() != SUCCESS)
        {
            LOG_INFO("ERROR4\n");
            return FAILURE;
        }

        FLASH_ROM_ERASE(MEASURENT_DATA_ADDR, MEASURENT_DATA_FLASH_SIZE);

        for(uint16_t i=0; i<20; ++i)
        {
            LOG_INFO("%d\n", i);
            FLASH_Erase_Sector(4096*i);
        }
    }

    return SUCCESS;
}

/**************************************************************************************************
 * @fn      HAL_ChecksumCalculate
 *
 * @brief   Calculate buffer checksum
 *
 * @param   p_buf - address of buffer
 *          len - length of buffer
 *
 * @return  Checksum value
 **************************************************************************************************/
uint8_t HAL_FlashChecksumCalculate(uint8_t *p_buf, uint16_t len)
{
    uint16_t i;
    uint8_t check_sum = 0;

    for (i = 0; i < len; ++i)
    {
        check_sum = (uint8_t)(check_sum + p_buf[i]);
    }

    return check_sum;
}

/**************************************************************************************************
 * @fn      HAL_FlashChecksumCheck
 *
 * @brief   Check buffer checksum
 *
 * @param   p_buf - address of buffer
 *          len - length of buffer
 *
 * @return  SUCCESS or FAILURE
 **************************************************************************************************/
uint8_t HAL_FlashChecksumCheck(uint8_t *p_buf, uint16_t len)
{
    uint16_t i;
    uint8_t check_sum = 0;

    for (i = 0; i < len; ++i)
    {
        check_sum = (uint8_t)(check_sum + p_buf[i]);
    }

    if (check_sum == p_buf[i])
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

/******************************** endfile @ bond ******************************/
