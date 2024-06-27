/********************************** (C) COPYRIGHT *******************************
 * File Name          : OTA.h
 * Author             : WCH
 * Version            : V1.10
 * Date               : 2018/12/14
 * Description        : oad相关配置定义
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#ifndef __OTA_H
#define __OTA_H

#define OTA_FLASH_ERASE_EVT     0x0004  //OTA Flash擦除任务
#define OTA_IAP_END_EVT         0x0010  //OTA_IAP_END_EVT
#define OTA_FLASH_PROM_EVT      0x0020  //OTA Flash
#define OTA_FLASH_VRIF_EVT      0x0040  //OTA Flash
#define OTA_RESTART_EVT         1<<8  //OTA RESTART

#define IAP_SAFE_FLAG       0x91600592

#define CHIP_ID             0x92
/* ------------------------------------------------------------------------------------------------
 *                                             OTA FLASH
 * ------------------------------------------------------------------------------------------------
 */

/* 整个用户code区分成四块，4K，216K，216K，12K，后三块下面分别叫做imageA（APP），imageB（OTA）和imageIAP */

/* FLASH定义 */
#define FLASH_BLOCK_SIZE       EEPROM_BLOCK_SIZE
#define IMAGE_SIZE             216 * 1024

/* imageA定义 */
#define IMAGE_A_FLAG           0x01
#define IMAGE_A_START_ADD      4 * 1024
#define IMAGE_A_SIZE           IMAGE_SIZE

/* imageB定义 */
#define IMAGE_B_FLAG           0x02
#define IMAGE_B_START_ADD      (IMAGE_A_START_ADD + IMAGE_SIZE)
#define IMAGE_B_SIZE           IMAGE_SIZE

/* imageIAP定义 */
#define IMAGE_IAP_FLAG         0x03
#define IMAGE_IAP_START_ADD    (IMAGE_B_START_ADD + IMAGE_SIZE)
#define IMAGE_IAP_SIZE         12 * 1024

/* IAP定义 */
#define IAP_STATE_SUCCESS      0x00               // IAP成功
#define IAP_STATE_RETRAN       0x01               // IAP重传
#define IAP_STATE_FAILURE      0xFF               // IAP失败

/* 以下为IAP下载命令定义 */
#define CMD_HAND_SHAKE         0x5A               // 握手命令
#define CMD_HAND_SHAKE_ACK     0xA5               // 握手ACK

#define CMD_IAP_PROM           0x80               // IAP编程命令
#define CMD_IAP_ERASE          0x81               // IAP擦除命令
#define CMD_IAP_VERIFY         0x82               // IAP校验命令
#define CMD_IAP_END            0x83               // IAP结束标志
#define CMD_IAP_INFO           0x84               // IAP获取设备信息
#define CMD_IAP_INFO_ACK       0x04               // IAP选择固件ACK

#define CMD_STATE_ACK          0x0F               // 状态ACK

/* 数据帧长度定义 */
#define IAP_LEN                64//36

#define IAP_SRC_RF             0x01
#define IAP_SRC_BLE            0x02
#define IAP_SRC_USB            0x03

/* 存放在DataFlash地址，不能占用蓝牙的位置 */
#define OTA_DATAFLASH_ADD      0x00077000 - FLASH_ROM_MAX_SIZE

/* 存放在DataFlash里的OTA信息 */
typedef struct
{
    unsigned char ImageFlag; //记录的当前的image标志
    unsigned char Revd[3];
} OTADataFlashInfo_t;

/* OTA IAP通讯协议定义 */
/* 地址使用4倍偏移 */
typedef union
{
    struct
    {
        unsigned char cmd;          /* 命令码 0x5A */
        unsigned char len;          /* 后续数据长度 */
        unsigned char string[7];    /* WCH@IAP */
    } handshake; /* 握手命令 */
    struct
    {
        unsigned char cmd;          /* 命令码 0x81 */
        unsigned char len;          /* 后续数据长度 */
        unsigned char addr[2];      /* 擦除地址 */
        unsigned char block_num[2]; /* 擦除块数 */

    } erase; /* 擦除命令 */
    struct
    {
        unsigned char cmd;       /* 命令码 0x83 */
        unsigned char len;       /* 后续数据长度 */
        unsigned char status[2]; /* 两字节状态，保留 */
    } end;                       /* 结束命令 */
    struct
    {
        unsigned char cmd;              /* 命令码 0x82 */
        unsigned char len;              /* 后续数据长度 */
        unsigned char addr[2];          /* 校验地址 */
        unsigned char buf[IAP_LEN - 4]; /* 校验数据 */
    } verify;                           /* 校验命令 */
    struct
    {
        unsigned char cmd;              /* 命令码 0x80 */
        unsigned char len;              /* 后续数据长度 */
        unsigned char addr[2];          /* 地址 */
        unsigned char buf[IAP_LEN - 4]; /* 后续数据 */
    } program;                          /* 编程命令 */
    struct
    {
        unsigned char cmd;              /* 命令码 0x84 */
        unsigned char len;              /* 后续数据长度 */
        unsigned char buf[IAP_LEN - 2]; /* 后续数据 */
    } info;                             /* 编程命令 */
    struct
    {
        unsigned char buf[IAP_LEN]; /* 接收数据包*/
    } other;
} OTA_IAP_CMD_t;

/* 记录当前的Image */
extern unsigned char CurrImageFlag;
extern uint8_t ota_taskID;
extern __attribute__((aligned(4))) uint8_t flash_buf[256];

void OTA_Init(void);

void OTA_Enable(void);
void OTA_USB_IAPWriteData(unsigned char *p_data, unsigned char w_len);

void OTA_RF_IAPWritData(unsigned char *p_data, unsigned char w_len);
#endif
