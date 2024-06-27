/********************************** (C) COPYRIGHT *******************************
 * File Name          : SLEEP.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        :
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include <RF.h>
#include <SLEEP.h>
#include "wchrf.h"
#include "CH59x_common.h"

/***************************************************
 * Global variables
 */
#ifndef SLEEP_RTC_MIN_TIME
#define SLEEP_RTC_MIN_TIME                  US_TO_RTC(1000)
#endif
#ifndef SLEEP_RTC_MAX_TIME
#define SLEEP_RTC_MAX_TIME                  (RTC_TIMER_MAX_VALUE - 1000 * 1000 * 30)
#endif
#ifndef WAKE_UP_RTC_MAX_TIME
#define WAKE_UP_RTC_MAX_TIME                US_TO_RTC(1600)
#endif

/*******************************************************************************
 * @fn          RF_LowPower
 *
 * @brief       启动睡眠
 *
 * @param       time  -  睡眠的时间
 *
 * @return      none.
 */
void RF_LowPower(uint32_t time)
{
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    uint32_t  time_curr,time_set;
    unsigned long irq_status;

    sys_safe_access_enable();
    R8_RTC_MODE_CTRL &= ~RB_RTC_TRIG_EN;  // 触发模式
    sys_safe_access_disable();              //
    SYS_DisableAllIrq(&irq_status);

    time_curr = RTC_GetCycle32k();
    if( time >= RTC_TIMER_MAX_VALUE - time_curr )
    {
        time_set = time - (RTC_TIMER_MAX_VALUE - time_curr);
    }
    else
    {
        time_set = time + time_curr;
    }
    RTC_SetTignTime(time_set);
    gSleepFlag = TRUE;
    SYS_RecoverIrq(irq_status);
    sys_safe_access_enable();
    R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  // 触发模式
    sys_safe_access_disable();              //

    // LOW POWER-SLEEP模式
    LowPower_Sleep( RB_PWR_RAM24K | RB_PWR_RAM2K | RB_PWR_EXTEND ); //只保留24+2K SRAM 供电
    HSECFG_Current(HSE_RCur_100); // 降为额定电流(低功耗函数中提升了HSE偏置电流)
#endif
}

/*******************************************************************************
 * @fn      RF_SleepInit
 *
 * @brief   配置睡眠唤醒的方式   - RTC唤醒，触发模式
 *
 * @param   None.
 *
 * @return  None.
 */
void RF_SleepInit(void)
{
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    sys_safe_access_enable();
    R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE; // RTC唤醒
    sys_safe_access_disable();              //
//    sys_safe_access_enable();
//    R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  // 触发模式
//    sys_safe_access_disable();              //
//    PFIC_EnableIRQ(RTC_IRQn);
#endif
}
