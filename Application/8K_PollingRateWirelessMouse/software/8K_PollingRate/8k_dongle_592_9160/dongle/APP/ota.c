/********************************** (C) COPYRIGHT *******************************
 * File Name          : OTA.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ch9160.h"
#include "HAL.h"
#include "OTA.h"
#include "nvs_flash.h"
#include "app.h"

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * CONSTANTS
 */

const uint8_t hand_shake_string[] = "WCH@TOOL";
const uint32_t hex_num = 2;

/* 用于APP判断文件有效性 */
__attribute__((aligned(4))) uint32_t save_Flag __attribute__((section(".ImageFlag"))) = IAP_SAFE_FLAG;

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8_t ota_taskID;
// OTA IAP VARIABLES
/* OTA通讯的帧 */
OTA_IAP_CMD_t iap_rec_data;

/* OTA解析结果 */
uint32_t OpParaDataLen = 0;
uint32_t OpAdd = 0;

/* flash的数据临时存储 */
__attribute__((aligned(8))) uint8_t block_buf[16];

/* Image跳转函数地址定义 */
typedef int (*pImageTaskFn)(void);
pImageTaskFn user_image_tasks;

/* Flash 擦除过程 */
uint32_t EraseAdd = 0;      //擦除地址
uint32_t EraseBlockNum = 0; //需要擦除的块数
uint32_t EraseBlockCnt = 0; //擦除的块计数

/* FLASH 校验过程 */
uint8_t VerifyStatus = 0;

uint32_t hex_index=0;

uint32_t iap_ok=0;

#define MAX_FLASH_BUFF_LEN      256
__attribute__((aligned(4))) uint8_t flash_buf[MAX_FLASH_BUFF_LEN];
uint8_t flash_buf_uesd_len = 0;
uint8_t flash_verify_flag = 0;

uint8_t ota_send_buf[DevEP4SIZE]={0};
/*********************************************************************
 * LOCAL FUNCTIONS
 */
uint16_t    OTA_ProcessEvent(uint8_t task_id, uint16_t events);
void        OTA_IAPReadDataComplete(unsigned char index);
uint8_t        OTA_IAPWriteData(unsigned char *p_data, unsigned char w_len);
uint8_t        Rec_OTA_IAP_DataDeal(void);
void        OTA_IAP_SendCMDDealSta(uint8_t deal_status);
uint8_t OTA_get_checksum( uint8_t *pData, uint8_t len );
void DisableAllIRQ(void);
void SwitchImageFlag(uint8_t new_flag);
/*********************************************************************
 * PROFILE CALLBACKS
 */


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OTA_Init
 *
 * @brief   OTA_Init
 *
 * @return  none
 */
void OTA_Init()
{
    ota_taskID = TMOS_ProcessEventRegister(OTA_ProcessEvent);
}

/*********************************************************************
 * @fn      OTA_ProcessEvent
 *
 * @brief   OTA_ProcessEvent
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t OTA_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //OTA_FLASH_ERASE_EVT
    if(events & OTA_FLASH_ERASE_EVT)
    {
        uint8_t status;

//        PRINT("ERASE:%08x num:%d\r\n", (int)(EraseAdd + EraseBlockCnt * FLASH_BLOCK_SIZE), (int)EraseBlockCnt);
//        status = FLASH_ROM_ERASE(EraseAdd + EraseBlockCnt * FLASH_BLOCK_SIZE, FLASH_BLOCK_SIZE);
        status = FLASH_ROM_ERASE(EraseAdd ,EraseBlockNum * FLASH_BLOCK_SIZE);

        /* 擦除失败 */
        if(status != SUCCESS)
        {
            OTA_IAP_SendCMDDealSta(status);
            return (events ^ OTA_FLASH_ERASE_EVT);
        }

//        EraseBlockCnt++;
//
//        /* 擦除结束 */
//        if(EraseBlockCnt >= EraseBlockNum)
        {
            PRINT("ERASE Complete\r\n");
            OTA_IAP_SendCMDDealSta(status);
            return (events ^ OTA_FLASH_ERASE_EVT);
        }

        return (events);
    }

    if(events & OTA_IAP_END_EVT)
    {
        /* 当前的是ImageA */
        /* 关闭当前所有使用中断，或者方便一点直接全部关闭 */
        DisableAllIRQ();

        /* 修改DataFlash，切换至ImageIAP */
        SwitchImageFlag(IMAGE_IAP_FLAG);

        SYS_ResetExecute();
        return (events ^ OTA_IAP_END_EVT);
    }

    if(events & OTA_RESTART_EVT)
    {
        SYS_ResetExecute();
        return (events ^ OTA_RESTART_EVT);
    }

    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      OTA_IAP_SendData
 *
 * @brief   OTA IAP发送数据，使用时限制20字节以内
 *
 * @param   p_send_data - 发送数据的指针
 * @param   send_len    - 发送数据的长度
 *
 * @return  none
 */
void OTA_IAP_SendData()
{
    app_relay_ota_to_uart();
}

/*********************************************************************
 * @fn      OTA_IAP_SendCMDDealSta
 *
 * @brief   OTA IAP执行的状态返回
 *
 * @param   deal_status - 返回的状态
 *
 * @return  none
 */
void OTA_IAP_SendCMDDealSta(uint8_t deal_status)
{

    ota_send_buf[0] = CMD_STATE_ACK;
    ota_send_buf[1] = 0x01;
    ota_send_buf[2] = deal_status;
    ota_send_buf[3] = OTA_get_checksum(ota_send_buf,3);

    OTA_IAP_SendData();
}

/*********************************************************************
 * @fn      OTA_IAP_CMDErrDeal
 *
 * @brief   OTA IAP异常命令码处理
 *
 * @return  none
 */
void OTA_IAP_CMDErrDeal(void)
{
    OTA_IAP_SendCMDDealSta(0xfe);
}

/*********************************************************************
 * @fn      SwitchImageFlag
 *
 * @brief   切换dataflash里的ImageFlag
 *
 * @param   new_flag    - 切换的ImageFlag
 *
 * @return  none
 */
void SwitchImageFlag(uint8_t new_flag)
{
    /* 读取第一块 */
    EEPROM_READ(OTA_DATAFLASH_ADD, (uint32_t *)&block_buf[0], 4);

    /* 擦除第一块 */
    EEPROM_ERASE(OTA_DATAFLASH_ADD, EEPROM_PAGE_SIZE);

    /* 更新Image信息 */
    block_buf[0] = new_flag;

    /* 编程DataFlash */
    EEPROM_WRITE(OTA_DATAFLASH_ADD, (uint32_t *)&block_buf[0], 4);
}

/*********************************************************************
 * @fn      DisableAllIRQ
 *
 * @brief   关闭所有的中断
 *
 * @return  none
 */
void DisableAllIRQ(void)
{
    SYS_DisableAllIrq(NULL);
}

/*********************************************************************
 * @fn      OTA_get_checksum
 *
 * @brief   OTA_get_checksum
 *
 * @return  none
 */
uint8_t OTA_get_checksum( uint8_t *pData, uint8_t len )
{
    int i;
    uint8_t checksum = 0;
    for( i = 0; i < len; i++ )
    {
        checksum += pData[i];
    }
    return checksum;
}

/*********************************************************************
 * @fn      Rec_OTA_IAP_DataDeal
 *
 * @brief   接收到OTA数据包处理
 *
 * @return  none
 */
uint8_t Rec_OTA_IAP_DataDeal(void)
{
    switch(iap_rec_data.other.buf[0])
    {
        /* 握手 */
        case CMD_HAND_SHAKE:
        {
            if( tmos_memcmp(iap_rec_data.handshake.string, hand_shake_string, 7))
            {
                PRINT("CMD_HAND_SHAKE \r\n");

                ota_send_buf[0] = CMD_HAND_SHAKE_ACK;
                ota_send_buf[1] = 0x04;
                ota_send_buf[2] = (uint8_t)(hex_num & 0xff);
                ota_send_buf[3] = (uint8_t)((hex_num >> 8) & 0xff);
                ota_send_buf[4] = (uint8_t)((hex_num >> 16) & 0xff);
                ota_send_buf[5] = (uint8_t)((hex_num >> 24) & 0xff);

                ota_send_buf[6] = OTA_get_checksum(ota_send_buf,6);

                /* 发送信息 */
                OTA_IAP_SendData();
            }
            else
            {
                PRINT("%s : %s\r\n",hand_shake_string,iap_rec_data.handshake.string);
                OTA_IAP_SendCMDDealSta(IAP_STATE_FAILURE);
            }
            break;
        }
        /* 编程 */
        case CMD_IAP_PROM:
        {
            if( hex_index == 1 )
            {
                uint8_t  status;

                // IAP协议的数据长度包含了地址两字节
                iap_rec_data.program.len -= 2;
                if(flash_buf_uesd_len==0)
                {
                    OpParaDataLen = iap_rec_data.program.len;
                    OpAdd = (uint32_t)(iap_rec_data.program.addr[0]);
                    OpAdd |= ((uint32_t)(iap_rec_data.program.addr[1]) << 8);
                    OpAdd = OpAdd * 16;
                    if( OpAdd == 0x1000 )
                    {
                        if((iap_rec_data.program.buf[4]|(iap_rec_data.program.buf[5]<<8)|
                            (iap_rec_data.program.buf[6]<<16)|(iap_rec_data.program.buf[7]<<24)) != IAP_SAFE_FLAG)
                        {
                            iap_ok = 0;
                            OTA_IAP_SendCMDDealSta(0xF1);
                        }
                    }

                    OpAdd += IMAGE_A_SIZE;

    //                PRINT("IAP_PROM: %08x len:%d \r\n", (int)OpAdd, (int)OpParaDataLen);
                    tmos_memcpy(flash_buf, iap_rec_data.program.buf, OpParaDataLen);
                    flash_buf_uesd_len += OpParaDataLen;
                    if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                    {
                        status = FLASH_ROM_WRITE(OpAdd, flash_buf, flash_buf_uesd_len);
                        if(status)
                        {
                            PRINT("IAP_PROM err \r\n");
                            iap_ok = 0;
                        }
                        flash_buf_uesd_len = 0;
                    }
//                    /* 当前是ImageA，直接编程 */
//                    status = FLASH_ROM_WRITE(OpAdd, iap_rec_data.program.buf, (uint16_t)OpParaDataLen);
//                    if(status)
//                    {
//                        PRINT("IAP_PROM err \r\n");
//                        iap_ok = 0;
//                    }
                }
                else {
                    OpParaDataLen = iap_rec_data.program.len;
                    tmos_memcpy(&flash_buf[flash_buf_uesd_len], iap_rec_data.program.buf, OpParaDataLen);
                    flash_buf_uesd_len += OpParaDataLen;
                    if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                    {
                        status = FLASH_ROM_WRITE(OpAdd, flash_buf, flash_buf_uesd_len);
                        if(status)
                        {
                            PRINT("IAP_PROM err \r\n");
                            iap_ok = 0;
                        }
                        flash_buf_uesd_len = 0;
                    }
                }
                OTA_IAP_SendCMDDealSta(status);
            }
            else
            {
                return HEX_NOT_LOCAL;
            }
            break;
        }
        /* 擦除 -- 蓝牙擦除由主机控制 */
        case CMD_IAP_ERASE:
        {
            if( hex_index == 1 )
            {
                // IAP协议的数据长度包含了地址两字节
                iap_rec_data.program.len -= 2;
                OpAdd = (uint32_t)(iap_rec_data.erase.addr[0]);
                OpAdd |= ((uint32_t)(iap_rec_data.erase.addr[1]) << 8);
                OpAdd = OpAdd * 16;

                OpAdd += IMAGE_A_SIZE;

                EraseBlockNum = (uint32_t)(iap_rec_data.erase.block_num[0]);
                EraseBlockNum |= ((uint32_t)(iap_rec_data.erase.block_num[1]) << 8);
                EraseAdd = OpAdd;
                EraseBlockCnt = 0;

                /* 检验就放在擦除里清0 */
                VerifyStatus = 0;
                iap_ok = 1;
                flash_verify_flag = 0;
                flash_buf_uesd_len = 0;

                PRINT("IAP_ERASE start:%08x num:%d\r\n", (int)OpAdd, (int)EraseBlockNum);

                if(EraseAdd < IMAGE_B_START_ADD || (EraseAdd + (EraseBlockNum) * FLASH_BLOCK_SIZE) > IMAGE_IAP_START_ADD)
                {
                    OTA_IAP_SendCMDDealSta(0xF0);
                }
                else
                {
                    /* 启动擦除 */
                    tmos_set_event(ota_taskID, OTA_FLASH_ERASE_EVT);
                }
            }
            else
            {
                return HEX_NOT_LOCAL;
            }
            break;
        }
        /* 校验 */
        case CMD_IAP_VERIFY:
        {
            if( hex_index == 1 )
            {
                // IAP协议的数据长度包含了地址两字节
                iap_rec_data.program.len -= 2;
                uint8_t  status = 0;
                if(flash_verify_flag ==0)
                {
                    if(flash_buf_uesd_len)
                    {
                        /* 当前是ImageA，直接编程 */
                        status = FLASH_ROM_WRITE(OpAdd, flash_buf, flash_buf_uesd_len);
                        if(status)
                        {
                            PRINT("FLASH_ROM_WRITE err \r\n");
                            iap_ok = 0;
                        }
                        flash_buf_uesd_len = 0;
                    }
                }
                VerifyStatus |= status;
                flash_verify_flag = 1;
                if(flash_buf_uesd_len==0)
                {
                    OpParaDataLen = iap_rec_data.verify.len;
                    OpAdd = (uint32_t)(iap_rec_data.verify.addr[0]);
                    OpAdd |= ((uint32_t)(iap_rec_data.verify.addr[1]) << 8);
                    OpAdd = OpAdd * 16;

                    OpAdd += IMAGE_A_SIZE;

//                    PRINT("V: %x l:%d\n", (int)OpAdd, (int)OpParaDataLen);
                    tmos_memcpy(flash_buf, iap_rec_data.verify.buf, OpParaDataLen);
                    flash_buf_uesd_len += OpParaDataLen;
                    if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                    {
                        /* 当前是ImageA，直接读取ImageB校验 */
                        status = FLASH_ROM_VERIFY(OpAdd, flash_buf, flash_buf_uesd_len);
                        if(status)
                        {
                            PRINT("IAP_VERIFY err \r\n");
                            iap_ok = 0;
                        }
                        flash_buf_uesd_len = 0;
                    }
                }
                else {
                    OpParaDataLen = iap_rec_data.verify.len;
                    tmos_memcpy(&flash_buf[flash_buf_uesd_len], iap_rec_data.verify.buf, OpParaDataLen);
                    flash_buf_uesd_len += OpParaDataLen;
                    if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                    {
                        /* 当前是ImageA，直接读取ImageB校验 */
                        status = FLASH_ROM_VERIFY(OpAdd, flash_buf, flash_buf_uesd_len);
                        if(status)
                        {
                            PRINT("IAP_VERIFY err \r\n");
                            iap_ok = 0;
                        }
                        flash_buf_uesd_len = 0;
                    }
                }
//                PRINT("IAP_VERIFY: %08x len:%d \r\n", (int)OpAdd, (int)OpParaDataLen);

                VerifyStatus |= status;
                OTA_IAP_SendCMDDealSta(VerifyStatus);
            }
            else
            {
                return HEX_NOT_LOCAL;
            }
            break;
        }
        /* 编程结束 */
        case CMD_IAP_END:
        {
            if( hex_index == 1 )
            {
                PRINT("IAP_END \r\n");

                if(iap_ok)
                {
                    OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);

                    /* 延迟复位 */
                    tmos_start_task(ota_taskID, OTA_IAP_END_EVT, 160);
                }
                else {
                    PRINT("IAP_END ERR \r\n");
                }
            }
            else
            {
                return HEX_NOT_LOCAL;
            }
            break;
        }
        case CMD_IAP_INFO:
        {
            hex_index = iap_rec_data.info.hex[0] |
                iap_rec_data.info.hex[1]<<8 |
                iap_rec_data.info.hex[2]<<16 |
                iap_rec_data.info.hex[3]<<24 ;
            PRINT("IAP_INFO %d\r\n",hex_index);

            if( hex_index == 1 )
            {

                ota_send_buf[0] = CMD_IAP_INFO_ACK;
                ota_send_buf[1] = 8;
                /* IMAGE_SIZE */
                ota_send_buf[2] = (uint8_t)(IMAGE_SIZE & 0xff);
                ota_send_buf[3] = (uint8_t)((IMAGE_SIZE >> 8) & 0xff);
                ota_send_buf[4] = (uint8_t)((IMAGE_SIZE >> 16) & 0xff);
                ota_send_buf[5] = (uint8_t)((IMAGE_SIZE >> 24) & 0xff);

                /* BLOCK SIZE */
                ota_send_buf[6] = (uint8_t)(FLASH_BLOCK_SIZE & 0xff);
                ota_send_buf[7] = (uint8_t)((FLASH_BLOCK_SIZE >> 8) & 0xff);

                ota_send_buf[8] = CHIP_ID&0xFF;
                ota_send_buf[9] = (CHIP_ID>>8)&0xFF;
                ota_send_buf[10] = OTA_get_checksum(ota_send_buf,10);

                /* 发送信息 */
                OTA_IAP_SendData();
            }
            else if( hex_index == 2 )
            {
                return HEX_NOT_LOCAL;
            }
            else {
                PRINT("？？？hex_index %d\r\n",hex_index);
                OTA_IAP_SendCMDDealSta(IAP_STATE_FAILURE);
            }
            break;
        }

        /* 单载波命令 */
        case CMD_SINGLE_CHANNEL:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(1))
            {
                PRINT("SINGLE_CHANNEL %d\r\n",iap_rec_data.other.buf[2]);
                RFIP_SingleChannel(iap_rec_data.other.buf[2]);
                RFIP_SetTxPower(LL_TX_POWEER_0_DBM);
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /* 单载波功率 */
        case CMD_SINGLE_POWER:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(1))
            {
                PRINT("SINGLE_POWER %d\r\n",iap_rec_data.other.buf[2]);
                RFIP_SetTxPower(iap_rec_data.other.buf[2]);
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /* 负载电容 */
        case CMD_CAPACITANCE:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(1))
            {
                PRINT("CAPACITANCE %d\r\n",iap_rec_data.other.buf[2]);
                nvs_flash_info.capacitance = iap_rec_data.other.buf[2];
                nvs_flash_store();
                HSECFG_Capacitance(nvs_flash_info.capacitance);
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /* CMD_RESTART */
        case CMD_RESTART:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(0))
            {
                PRINT("CMD_RESTART\n");
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
                tmos_start_task(ota_taskID, OTA_RESTART_EVT, 160);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        default:
        {
            OTA_IAP_CMDErrDeal();
            break;
        }
    }
    return 0;
}

/*********************************************************************
 * @fn      OTA_IAPReadDataComplete
 *
 * @brief   OTA 数据读取完成处理
 *
 * @param   index   - OTA 通道序号
 *
 * @return  none
 */
void OTA_IAPReadDataComplete(unsigned char index)
{
    PRINT("OTA Send Comp \r\n");
}

/*********************************************************************
 * @fn      OTA_IAPWriteData
 *
 * @brief   OTA 通道数据接收完成处理
 *
 * @param   index   - OTA 通道序号
 * @param   p_data  - 写入的数据
 * @param   w_len   - 写入的长度
 *
 * @return  none
 */
uint8_t OTA_IAPWriteData(unsigned char *p_data, unsigned char w_len)
{
    if(p_data[p_data[1]+2] != OTA_get_checksum( p_data, p_data[1]+2 ))
    {
        OTA_IAP_SendCMDDealSta(IAP_STATE_RETRAN);
        return 0;
    }
    else
    {
        tmos_memcpy((unsigned char *)&iap_rec_data, p_data, p_data[1]+3);
        return Rec_OTA_IAP_DataDeal();
    }
}

/*********************************************************************
*********************************************************************/
