/********************************** (C) COPYRIGHT *******************************
 * File Name          : ota.h
 * Author             : WCH
 * Version            : V1.10
 * Date               : 2018/12/14
 * Description        : oad������ö���
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#ifndef __OTA_H
#define __OTA_H

/* ------------------------------------------------------------------------------------------------
 *                                             OTA FLASH
 * ------------------------------------------------------------------------------------------------
 */

/* �����û�code���ֳ��Ŀ飬����Ϊ4K��44K��16K��128K ������ֱ����imageA��APP����imageB��IAP����LIB */

/* FLASH���� */

#define FLASH_BLOCK_SIZE     EEPROM_BLOCK_SIZE

/* imageA���� */
#define IMAGE_A_FLAG         0x01
#define IMAGE_A_START_ADD    0x1000
#define IMAGE_A_SIZE         44 * 1024

/* imageB���� */
#define IMAGE_B_FLAG         0x02
#define IMAGE_B_START_ADD    (IMAGE_A_START_ADD + IMAGE_A_SIZE)
#define IMAGE_B_SIZE         16 * 1024

#define IMAGE_OTA_FLAG       0x03

#define jumpApp              ((void (*)(void))((uint32_t *)IMAGE_A_START_ADD))

/* IAP���� */
/* ����ΪIAP��������� */
#define CMD_IAP_PROM         0x80               // IAP�������
#define CMD_IAP_ERASE        0x81               // IAP��������
#define CMD_IAP_VERIFY       0x82               // IAPУ������
#define CMD_IAP_END          0x83               // IAP������־
#define CMD_IAP_INFO         0x84               // IAP��ȡ�豸��Ϣ

/* ����֡���ȶ��� */
#define IAP_LEN              247

/* �����DataFlash��ַ������ռ��������λ�� */
#define OTA_DATAFLASH_ADD    0x00076000 - FLASH_ROM_MAX_SIZE

/* �����DataFlash���OTA��Ϣ */
typedef struct
{
    unsigned char ImageFlag; //��¼�ĵ�ǰ��image��־
    unsigned char Revd[3];
} OTADataFlashInfo_t;

/* OTA IAPͨѶЭ�鶨�� */
/* ��ַʹ��4��ƫ�� */
typedef union
{
    struct
    {
        unsigned char cmd;          /* ������ 0x81 */
        unsigned char len;          /* �������ݳ��� */
        unsigned char addr[2];      /* ������ַ */
        unsigned char block_num[2]; /* �������� */

    } erase; /* �������� */
    struct
    {
        unsigned char cmd;       /* ������ 0x83 */
        unsigned char len;       /* �������ݳ��� */
        unsigned char status[2]; /* ���ֽ�״̬������ */
    } end;                       /* �������� */
    struct
    {
        unsigned char cmd;              /* ������ 0x82 */
        unsigned char len;              /* �������ݳ��� */
        unsigned char addr[2];          /* У���ַ */
        unsigned char buf[IAP_LEN - 4]; /* У������ */
    } verify;                           /* У������ */
    struct
    {
        unsigned char cmd;              /* ������ 0x80 */
        unsigned char len;              /* �������ݳ��� */
        unsigned char addr[2];          /* ��ַ */
        unsigned char buf[IAP_LEN - 4]; /* �������� */
    } program;                          /* ������� */
    struct
    {
        unsigned char cmd;              /* ������ 0x84 */
        unsigned char len;              /* �������ݳ��� */
        unsigned char buf[IAP_LEN - 2]; /* �������� */
    } info;                             /* ������� */
    struct
    {
        unsigned char buf[IAP_LEN]; /* �������ݰ�*/
    } other;
} OTA_IAP_CMD_t;

/* ��¼��ǰ��Image */
extern unsigned char CurrImageFlag;

#endif
