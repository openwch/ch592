/********************************** (C) COPYRIGHT *******************************
* File Name          : rf.h
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/10
* Description        : 
* Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
*******************************************************************************/

#ifndef __RF_H
#define __RF_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "CH59x_common.h"
#include "wchrf.h"


#define US_TO_TICK(us)                  (uint32_t)((us)/(1000000/(CAB_LSIFQ)))
#define SLEEP_WAIT_HSE_TIME             US_TO_TICK(2400)
#define RTC_TIMER_MAX_VALUE             0xA8C00000

#ifdef CLK_OSC32K
#if (CLK_OSC32K==1)
#define FREQ_RTC    32000
#else
#define FREQ_RTC    32768
#endif
#endif /* CLK_OSC32K */

#define CLK_PER_US                  (1.0 / ((1.0 / FREQ_RTC) * 1000 * 1000))
#define CLK_PER_MS                  (CLK_PER_US * 1000)

#define US_PER_CLK                  (1.0 / CLK_PER_US)
#define MS_PER_CLK                  (US_PER_CLK / 1000.0)

#define RTC_TO_US(clk)              ((uint32_t)((clk) * US_PER_CLK + 0.5))
#define RTC_TO_MS(clk)              ((uint32_t)((clk) * MS_PER_CLK + 0.5))

#define US_TO_RTC(us)               ((uint32_t)((us) * CLK_PER_US + 0.5))
#define MS_TO_RTC(ms)               ((uint32_t)((ms) * CLK_PER_MS + 0.5))

#define  PHY_MODE      PHY_MODE_PHY_2M

#define  ALIGNED4(x)       ((x+3)/4*4)

#define  RF_RXBUFNB        16
#define  RF_TXBUFNB        16  // 不能小于8

#define  RF_RX_BUF_SZE     64
#define  RF_TX_BUF_SZE     64

extern  RF_DMADESCTypeDef *pDMARxGet;
extern  RF_DMADESCTypeDef *pDMATxGet;
extern tmosTaskID rfTaskID;
extern volatile uint8_t  gSleepFlag;

void rfDMADescInit( void );
void RF_LibInit( pfnRfRoleProcess cb );
void HAL_TimeInit();
void RTC_SetTignTime(uint32_t time);
void Lib_Calibration_LSI(void);
void RF_ProcessCallBack( rfRole_States_t sta,uint8_t id );

#ifdef __cplusplus
}
#endif

#endif
