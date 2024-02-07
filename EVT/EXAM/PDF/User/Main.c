/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/08/08
 * Description        : Udisk_PDF Main.c
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* Header Files */
#include "CH59x_common.h"
#include "usbfs_device.h"
#include "SW_UDISK.h"
#include "Internal_Flash.h"
#include "SPI_FLASH.h"
#include "CHRV3UFI.h"
#include "pdfTempe.h"


#define CONFIG_MAIN_DEBUG
#ifdef CONFIG_MAIN_DEBUG
#define LOG_INFO(...)       printf(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif


/*********************************************************************
 * @fn      Uart_Init
 *
 * @brief   Initialize serial port
 *
 * @return  none
 */
void Uart_Init(void)
{
    /* Configure serial port 1: First configure the IO port mode, then configure the serial port */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);      // RXD-Configure pull-up input
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); // TXD-Configure push-pull output, paying attention to first allowing the IO port to output a high level
    UART1_DefInit();
}


/*********************************************************************
 * @fn      main
 *
 * @brief   main
 *
 * @return  none
 */
int main()
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);              // The system running clock is configured to 60MHz
    Uart_Init();                                    // Serial port 1 initialization
    LOG_INFO("WCH pdf create demo program.Complair @ %s,%s.\r\n",__DATE__,__TIME__);   // Print the current time, provided by the compiler

#if  FUN_UDISK
#if (STORAGE_MEDIUM == MEDIUM_SPI_FLASH)
    LOG_INFO( "USBD UDisk Demo\r\nStorage Medium: SPI FLASH \r\n" );
    /* SPI flash init */
    FLASH_Port_Init( );
    /* FLASH ID check */
    FLASH_IC_Check( );
#elif (STORAGE_MEDIUM == MEDIUM_INTERAL_FLASH)      // Storage location is internal Flash
    LOG_INFO( "USBD UDisk Demo\r\nStorage Medium: INTERNAL FLASH \r\n" );
    Flash_Sector_Count = IFLASH_UDISK_SIZE  / DEF_UDISK_SECTOR_SIZE;             // Number of Flash sectors: Internal Flash allocation space/size of each sector on the USB drive
    Flash_Sector_Size = DEF_UDISK_SECTOR_SIZE;                                   // Flash sector size
    LOG_INFO("Flash_Sector_Count:%d | Flash_Sector_Size:%d\r\n",Flash_Sector_Count,Flash_Sector_Size);
#endif
    /* Enable Udisk */
    Udisk_Capability = Flash_Sector_Count;
    Udisk_Status |= DEF_UDISK_EN_FLAG;              // USB drive status
#endif

#if  FUN_FILE_CREATE                                // Configurable FUN_FILE_CREATE in engineering attribute pre compilation
  {
    uint8_t state;

    /* USB Libs Initialization */
    printf( "UDisk library Initialization. \r\n" );
    state = CHRV3LibInit( );                        // Initialize the CHRV3 library, successful operation returns 0
    if(state == ERR_SUCCESS)
    {
        printf( "LIB Init SUCCESS\r\n");
    }
    CHRV3DiskStatus = DISK_MOUNTED;
  }
#endif

    pdf_create( "TEMP.PDF" );

    while(1);
}

