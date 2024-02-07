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
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "flash_info.h"
#include "usb_connect.h"
#include "internal_flash.h"
#include "spi_flash.h"
#include "sw_udisk.h"

//#define CONFIG_MAIN_DEBUG
#ifdef CONFIG_MAIN_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

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
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
    }
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
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);
//#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
//#endif
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

    /* Check if the USB is connected */
    DeviceStatus = DeviceInfo.StatusFlag;

    LOG_INFO("%s\n", VER_LIB);
    CH59x_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    Main_Circulation();
}


//__INTERRUPT
//__HIGH_CODE
//void GPIOA_IRQHandler(void)
//{
////    KeyPollEnFlag = 1;
//    GPIOA_ClearITFlagBit(KEY1_BV|USB_CHECK_BV);
//}

/******************************** endfile @ main ******************************/
