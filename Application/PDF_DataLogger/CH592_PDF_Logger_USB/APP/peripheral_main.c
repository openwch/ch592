/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外设从机应用主函数及任务系统初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CH59x_common.h"
#include "peripheral.h"
#include "flash_info.h"
#include "usb_connect.h"
#include "internal_flash.h"
#include "spi_flash.h"
#include "sw_udisk.h"
#include "CHRV3UFI.h"
#include "pdfTempe.h"
#include "pdfFile.h"

//#define CONFIG_MAIN_DEBUG
#ifdef CONFIG_MAIN_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

#define IMAGE_A_START_ADD      0x14000
#define jumpApp    ((void (*)(void))((int *)IMAGE_A_START_ADD))

/* Status of device */
#define DEF_DEVICE_STATUS_FACTORY   0x00
#define DEF_DEVICE_STATUS_NORMAL    0x01
#define DEF_DEVICE_STATUS_USB       0x02

#define IFLASH_TEMP_START_ADDR   (268*1024)
#define IFLASH_HUMI_START_ADDR   (358*1024)
/*********************************************************************
 * GLOBAL TYPEDEFS
 */

//__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];
__attribute__((aligned(4))) RAM_BUFFER_t RAM_BUFFER;

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/* Status of device */
uint8_t DeviceStatus;

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */

float AHT20_TemperatureValue;
float AHT20_HumidityValue;

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
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);

#ifdef CONFIG_MAIN_DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    LOG_INFO("%s\r\n", __TIME__);

#if (STORAGE_MEDIUM == MEDIUM_SPI_FLASH)
        LOG_INFO( "USBD UDisk Demo\r\nStorage Medium: SPI FLASH \r\n" );
        /* SPI flash init */
        FLASH_Port_Init( );
        /* FLASH ID check */
        FLASH_IC_Check( );
#elif (STORAGE_MEDIUM == MEDIUM_INTERAL_FLASH)
        LOG_INFO( "USBD UDisk Demo\r\nStorage Medium: INTERNAL FLASH \r\n" );
        Flash_Sector_Count = IFLASH_UDISK_SIZE  / DEF_UDISK_SECTOR_SIZE;
        Flash_Sector_Size = DEF_UDISK_SECTOR_SIZE;
        LOG_INFO("Flash_Sector_Count:%d | Flash_Sector_Size:%d\r\n",Flash_Sector_Count,Flash_Sector_Size);
#endif

    /* Read device information */
    HAL_ReadDeviceInfo();

    LOG_INFO("Bond Info:");
    for (uint8_t i = 0; i < DEVICEINFO_LEN; ++i)
    {
        LOG_INFO(" %02x", ((uint8_t * )(&DeviceInfo))[i]);
    }
    LOG_INFO("\n");
    LOG_INFO("NO.%d | StatusFlag:%d | MeasureNum:%d\n", DeviceInfo.InfoNum, DeviceInfo.StatusFlag, DeviceInfo.MeasureNum);
    LOG_INFO("MeasureInterval:%d | Unit:%d\n", DeviceInfo.PdfParam.MeasureInterval, DeviceInfo.PdfParam.TempUnit);
    LOG_INFO("MaxTempAlarm:%f | MinTempAlarm:%f\n", DeviceInfo.PdfParam.MaxTempAlarm, DeviceInfo.PdfParam.MinTempAlarm);
    LOG_INFO("MaxHumiAlarm:%f | MinHumiAlarm:%f\n", DeviceInfo.PdfParam.MaxHumiAlarm, DeviceInfo.PdfParam.MinHumiAlarm);

//    ++DeviceInfo.MeasureNum;
//    ++DeviceInfo.MeasureNum;
//    ++DeviceInfo.MeasureNum;
//    HAL_SaveDeviceInfo();
//    HAL_ReadDeviceInfo();
//    LOG_INFO("NO.%d | MeasureNum:%d\n", DeviceInfo.InfoNum, DeviceInfo.MeasureNum);
//    DeviceInfo.MeasureNum=0;
//    HAL_SaveDeviceInfo();
//
//    for(uint8_t i=0; i<100; ++i)
//    {
//        AHT20_TemperatureValue = i*1.0 - 25.0;
//        AHT20_HumidityValue = i*1.0;
//        FLASH_ROM_WRITE(IFLASH_TEMP_START_ADDR+4*DeviceInfo.MeasureNum, &AHT20_TemperatureValue, 4);
//        FLASH_ROM_WRITE(IFLASH_HUMI_START_ADDR+4*DeviceInfo.MeasureNum, &AHT20_HumidityValue, 4);
//        ++DeviceInfo.MeasureNum;
//        HAL_SaveDeviceInfo();
//    }
//
//    LOG_INFO("NO.%d | MeasureNum:%d\n", DeviceInfo.InfoNum, DeviceInfo.MeasureNum);
//    for(uint8_t i=0; i<100; ++i)
//    {
//        FLASH_ROM_READ(IFLASH_TEMP_START_ADDR+4*i, &AHT20_TemperatureValue, 4);
//        FLASH_ROM_READ(IFLASH_HUMI_START_ADDR+4*i, &AHT20_HumidityValue, 4);
//        PRINT("%d: %f\n", i, AHT20_TemperatureValue);
//        PRINT("%d: %f\n", i, AHT20_HumidityValue);
//    }

    /* Check if the USB is connected */
    GPIOA_ModeCfg(USB_CHECK_BV, GPIO_ModeIN_PD);
    if(USB_CONNECT_CHECK() != 0)
    {
        DeviceInfo.StatusFlag = DEF_DEVICE_STATUS_FACTORY;
        HAL_SaveDeviceInfo();
//        DeviceStatus = DEF_DEVICE_STATUS_USB;

#if  FUN_UDISK
        /* Enable Udisk */
        Udisk_Capability = Flash_Sector_Count;
        Udisk_Status |= DEF_UDISK_EN_FLAG;
#endif

#if  FUN_FILE_CREATE
    {
        uint8_t state;

        /* USB Libs Initialization */
        LOG_INFO( "UDisk library Initialization. \r\n" );
        state = CHRV3LibInit( ); //Initialize the file system library, the sector size set by DEMO is 512B
        if(state == ERR_SUCCESS)
        {
            LOG_INFO( "LIB Init SUCCESS\r\n");
        }
        CHRV3DiskStatus = DISK_MOUNTED;
    }
#endif

        pdf_create( "TEMP.PDF" ); //Create pdf file, the file name can be changed

        while(1)
        {
            if(USB_CONNECT_CHECK() == 0)
            {
                DelayMs(10);
                SYS_ResetExecute();
            }
        }
    }
    else
    {
        LOG_INFO("jump\n");
        DelayMs(10);
        jumpApp();
    }


}


/******************************** endfile @ main ******************************/
