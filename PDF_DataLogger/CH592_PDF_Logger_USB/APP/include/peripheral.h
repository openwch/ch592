/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/11
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Peripheral Task Events
#define SBP_START_DEVICE_EVT    0x0001
#define SBP_PERIODIC_EVT        0x0002
#define SBP_READ_RSSI_EVT       0x0004
#define SBP_PARAM_UPDATE_EVT    0x0008
#define SBP_PHY_UPDATE_EVT      0x0010
#define SBP_ENABLE_ADV_EVT      0x0020
#define SBP_DISABLE_ADV_EVT     0x0040

/*********************************************************************
 * MACROS
 */
typedef struct
{
    uint16_t connHandle; // Connection handle of current connection
    uint16_t connInterval;
    uint16_t connSlaveLatency;
    uint16_t connTimeout;
} peripheralConnItem_t;

extern uint8_t Peripheral_TaskID; // Task ID for internal task/event processing


//#include "CONFIG.h"
#include "pdfFile.h"
#include "sw_udisk.h"
#include "CHRV3UFI.h"

typedef struct
{
    uint8_t pdf_data_buf[PDF_TMP_BUF_LEN_MAX+PDF_TMP_BUF_LEN_EXT];
    uint8_t UDisk_Down_Buffer[DEF_FLASH_SECTOR_SIZE];
    uint8_t UDisk_Pack_Buffer[DEF_UDISK_PACK_64];
    uint8_t DISK_BASE_BUF[DISK_BASE_BUF_LEN]; /* 外部RAM的磁盘数据缓冲区,缓冲区长度为一个扇区的长度 */
    uint8_t DISK_FAT_BUF[ DISK_BASE_BUF_LEN ]; /* 外部RAM的磁盘FAT数据缓冲区,缓冲区长度为一个扇区的长度 */
}PDF_BUFFER_t;


typedef union
{
//    uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];
    PDF_BUFFER_t PDF_BUFFER;

}RAM_BUFFER_t;

extern RAM_BUFFER_t RAM_BUFFER;


/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void Peripheral_Init(void);

/*
 * Task Event Processor for the BLE Application
 */
extern uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
