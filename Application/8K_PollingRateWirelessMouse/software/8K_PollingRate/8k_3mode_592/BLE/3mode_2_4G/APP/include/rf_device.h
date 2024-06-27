/********************************** (C) COPYRIGHT *******************************
* File Name          : rf_device.h
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/10
* Description        : 
*******************************************************************************/

#ifndef __RF_TEST_H
#define __RF_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "wchrf.h"

#define  DEVICE_KEYBOARD                  0
#define  DEVICE_MOUSE                     1

#define  DEVICE_KEYBOARD_TYPE                  1
#define  DEVICE_MOUSE_TYPE                     2

#define RF_START_BOUND_EVENT     (1<<0)
#define RF_SLEEP_EVENT         (1<<1)
#define RF_SLEEP_TRIGGER_EVENT         (1<<2)
#define RF_RECV_PROCESS_EVENT         (1<<3)
#define RF_REPORT_DISCONNECT_EVENT         (1<<4)

#define RF_TEST_DATA_EVENT       (1<<12)
#define RF_TEST_RX_EVENT         (1<<13)
#define RF_TEST_TX_EVENT         (1<<14)

#define  PHY_MODE      PHY_MODE_PHY_2M

#define   TX_DATA_MAX_LEN          255
#define   TX_DATA_TEST_TYPE        0xFF
#define   TX_DATA_TEST_LEN         100

//#define   TX_TIM_TX_INTERVAL       (1000)  // Xus

#ifndef TX_TIM_TX_INTERVAL
#if(PHY_MODE  == PHY_MODE_1M )
#define   TX_TIM_TX_INTERVAL       ((8+32+(TX_DATA_TEST_LEN*8)+24)+120)  // Xus
#else
#define   TX_TIM_TX_INTERVAL       ( ((16+32+(TX_DATA_TEST_LEN*8)+24)/2)+50)  // Xus
#endif
#endif

#define   TX_TOAL_COUNT            (1000000/TX_TIM_TX_INTERVAL)  // 1s时间可发送的数量


typedef enum
{
    RF_CON_IDEL = 0,
    RF_CON_PAIRING,
    RF_CON_CONNECTED,
}RF_con_status_t;


extern RF_DMADESCTypeDef *pDMARxGet;
extern RF_DMADESCTypeDef *pDMATxGet;

void RF_Tx( void );
void RF_Rx( void );
void RF_Init(void);
uint8_t RF_check_con_status(RF_con_status_t status);
uint8_t rf_send_data( uint8_t *pData, uint8_t len);
void rfDMADescInit( void );
void rf_safe_flag_enable(void);
void rf_safe_flag_disable(void);
uint8_t rf_get_txbuf_num( void );
void RF_ReStart( void );
void RF_stop( void );
#ifdef __cplusplus
}
#endif

#endif
