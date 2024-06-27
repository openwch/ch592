/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch9160.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/11/12
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef ch9160_H
#define ch9160_H

#include "stdint.h"
#ifdef __cplusplus
"C" {
#endif

// ACCESS_CMD超时10ms
#define ACCESS_CMD_TIMEOUT      16

// ACCESS_STATE
#define STATE_SUCCESS           0x00
#define STATE_PENDING           0xFF

#define INTERFAVE_RECV_BUF_LEN  256
#define INTERFAVE_TRAN_BUF_LEN  256

#define CMD_ACK                 0x80
#define CMD_ACK_LEN             0x07

#define CHK_DATA                0x55
#define CHK_ACK                 0xAA

// 基本数据
#define CMD_CHK_CONNECT         0x01
#define CMD_GET_INFO            0x02
#define CMD_SET_INFO            0x03
#define CMD_GET_USB_DESC        0x04
#define CMD_SET_USB_DESC        0x05
#define CMD_SNED_ENDP_DATA1     0x06
#define CMD_SNED_ENDP_DATA2     0x07
#define CMD_RESET               0x08
#define CMD_AUTO_SEND_STATUS    0x89
#define CMD_AUTO_SEND_PC_DATA   0x8A

#define CMD_MRAK_MAX            0x8F
#define CMD_MRAK_ENDP           0x70

#define STATUS_ENUM_SUCCESS         0x00
#define STATUS_SET_REPORT_CHANGE    0x01
#define STATUS_RECV_SET_REPORT      0x02
#define STATUS_RECV_GET_REPORT      0x03
#define STATUS_RECV_BUF_FULL        0x04
#define STATUS_SLEEP                0x05

#define ENTER_SLEEP                 0x55
#define EXIT_SLEEP                  0xAA

#define USB_WEAK_UP_IO              (GPIO_Pin_12)
#define USB_WEAK_UP()               GPIOB_SetBits(USB_WEAK_UP_IO)
#define USB_SLEEP()                 GPIOB_ResetBits(USB_WEAK_UP_IO)

#define  VER_CH9160_FILE            "CH9160_LIB_V1.0"
extern const uint8_t VER_CH9160_LIB[];

typedef void (*access_cmd_cb_t)(uint8_t state, uint8_t cmd, uint8_t *pData, uint16_t len);

/**
 * @brief   注册接入层命令接收回调
 *
 * @param   cb - 回调函数
 */
void access_register_cmd_cb(access_cmd_cb_t cb);

/**
 * @brief   检查USB连接状态
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_chk_connect(void);

/**
 * @brief   检查USB连接状态
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_get_info(void);

/**
 * @brief   设置USB
 *
 * @param   usb_enable  : 1 - 使能USB; 0 - 关闭USB
 *          io_dir      : IO方向 0表示输入; 1表示输出
 *          io_pin      : IO电平 0表示低电平; 1表示高电平
 *          endpx_size  : 0~9对应端点长度为2的0~9次幂，比如2对应长度为4，6对应长度为64，9对应长度为512
 *          sleep_off   : 2 - USB深度睡眠; 1 - 关闭USB睡眠; 0 - USB浅睡眠
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_set_info(uint8_t usb_enable, uint8_t io_dir, uint8_t io_pin,
    uint8_t endp1_size, uint8_t endp2_size, uint8_t endp3_size, uint8_t endp4_size, uint8_t sleep_off);

/**
 * @brief   获取USB描述符
 *
 * @param   desc_type   : 0x00    USB设备描述符
 *                        0x01    USB配置描述符
 *                        0x02    USB HID1报表描述符
 *                        0x03    USB HID2报表描述符
 *                        0x04    USB HID3报表描述符
 *                        0x05    USB HID4报表描述符
 *                        0x06    USB HID5报表描述符
 *                        0x07    USB字符串0(语言)描述符
 *                        0x08    USB字符串1(厂商)描述符
 *                        0x09    USB字符串2(产品)描述符
 *                        0x0A    USB字符串3(序列号)描述符
 *                        0x0B    USB字符串4描述符
 *          offset      : USB描述符偏移地址
 *          length      : USB描述符数据长度
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_get_usb_desc(uint8_t desc_type, uint16_t offset, uint16_t length);

/**
 * @brief   设置USB描述符
 *
 * @param   desc_type   : 0x00    USB设备描述符
 *                        0x01    USB配置描述符
 *                        0x02    USB HID1报表描述符
 *                        0x03    USB HID2报表描述符
 *                        0x04    USB HID3报表描述符
 *                        0x05    USB HID4报表描述符
 *                        0x06    USB HID5报表描述符
 *                        0x07    USB字符串0(语言)描述符
 *                        0x08    USB字符串1(厂商)描述符
 *                        0x09    USB字符串2(产品)描述符
 *                        0x0A    USB字符串3(序列号)描述符
 *                        0x0B    USB字符串4描述符
 *          offset      : USB描述符偏移地址
 *          length      : USB描述符数据长度
 *          pData       : USB描述符数据
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_set_usb_desc(uint8_t desc_type, uint16_t offset, uint16_t length, uint8_t *pData);

/**
 * @brief   指定端点发送数据，发送后需等待收到应答后再发送新的数据
 *
 * @param   endp    : 端点号，支持1~4
 *          pData   : 数据指针
 *          length  : 数据长度
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_send_endp_data( uint8_t endp, uint8_t *pData, uint16_t length);

/**
 * @brief   指定端点发送数据，无需等待应答
 *
 * @param   endp    : 端点号，支持1~4
 *          pData   : 数据指针
 *          length  : 数据长度
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_send_endp_data_without_ack( uint8_t endp, uint8_t *pData, uint16_t length);

/**
 * @brief   复位USB
 *
 * @param   reset_type :  0x00：芯片整体复位
 *                        0x01：芯片仅USB复位
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_usb_reset(uint8_t reset_type);

/**
 * @brief   收到USB主动上报的命令后需要发送应答
 *
 * @param   cmd :  接收到的USB主动上报的命令码
 *
 * @return  @ACCESS_STATE.
 */
uint8_t access_send_ack(uint8_t cmd);

/**
 * @brief   USB数据处理
 */
void trans_process(void);

/**
 * @brief   重传上一包发送给USB的数据
 *
 * @return  @ACCESS_STATE.
 */
uint8_t trans_retran_last_data(void );

/**
 * @brief   接入层初始化
 */
void access_Init( void );

/**
 * @brief   CH9160初始化
 */
void ch9160_Init( void );


#ifdef __cplusplus
}
#endif

#endif
