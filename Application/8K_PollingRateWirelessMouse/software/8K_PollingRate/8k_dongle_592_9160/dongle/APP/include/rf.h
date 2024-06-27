/********************************** (C) COPYRIGHT *******************************
* File Name          : rf_test.h
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
#include "ch9160.h"
#include "wchrf.h"

#define START_PEER_BOUND_DELAY      1600*3
#define RF_START_ALL_BOUND_EVENT     (1<<0)
#define RF_START_PEER_BOUND_EVENT     (1<<1)


#define RF_TEST_DATA_EVENT       (1<<12)
#define RF_TEST_RX_EVENT         (1<<13)
#define RF_TEST_TX_EVENT         (1<<14)
#define RF_TEST_TTX_EVENT        (1<<11)

#define DEFAULT_PAIR_ACCESS_ADDE    0x71763988

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
extern uint32_t gUartCount;
extern uint32_t gTxCount;
void RF_Tx( void );
void RF_Rx( void );
void RF_Init(void);
uint8_t *rf_get_data( uint8_t *pLen);
void rf_delete_data(void);
uint8_t rf_send_data( uint8_t *pData, uint8_t len);


#ifdef __cplusplus
}
#endif

#endif
