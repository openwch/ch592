/********************************** (C) COPYRIGHT *******************************
* File Name          : trans.h
* Author             : Hikari
* Version            : V1.0
* Date               : 2024/01/18
* Description        : 
*******************************************************************************/

#ifndef __trans_H
#define __trans_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SBP_TRANS_USB_EVT       1<<0
#define SBP_ENTER_SLEEP_EVT      1<<1
#define SBP_ENTER_SLEEP_DELAY_EVT      1<<2

extern uint8_t tran_taskID;

void trans_Init( void );
uint8_t trans_send_data( uint8_t *pData, uint8_t len );

#ifdef __cplusplus
}
#endif

#endif
