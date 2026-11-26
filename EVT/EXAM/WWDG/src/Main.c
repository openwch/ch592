/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : 窗口看门狗WDOG例程：溢出中断喂狗演示与超时复位演示
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

/* 喂狗周期：需小于看门狗溢出时间 */
#define WDOG_FEED_MS      100

/* 喂狗次数：喂够后停止喂狗，演示看门狗复位 */
#define WDOG_FEED_TIMES   5

/* 溢出中断次数：中断模式下发生3次溢出后退出 */
#define WDOG_INT_TIMES    3

volatile uint8_t wdog_int_cnt = 0;

/*********************************************************************
 * @fn      DebugInit
 *
 * @brief   调试初始化
 *
 * @return  none
 */
void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

/*********************************************************************
 * @fn      PrintResetCause
 *
 * @brief   打印最近一次复位原因
 *
 * @return  none
 */
void PrintResetCause(void)
{
    switch(SYS_GetLastResetSta())
    {
        case RST_STATUS_SW:
            PRINT("  reset cause: software reset SR\n");
            break;
        case RST_STATUS_RPOR:
            PRINT("  reset cause: power-on reset RPOR\n");
            break;
        case RST_STATUS_WTR:
            PRINT("  reset cause: watchdog timeout reset WTR\n");
            break;
        case RST_STATUS_MR:
            PRINT("  reset cause: external pin reset MR\n");
            break;
        case RST_STATUS_GPWSM:
            PRINT("  reset cause: power-down wake-up reset GPWSM\n");
            break;
        default:
            PRINT("  reset cause: code 0x%x\n", SYS_GetLastResetSta());
            break;
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main(void)
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* 配置串口调试 */
    DebugInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);
    PRINT("WWDG example: interrupt feed then timeout reset\n");

    uint8_t feeds = 0;

    /* 配置串口调试 */
    DebugInit();
    PRINT("Time:%s; Start @ChipID=%02X\n", __TIMESTAMP__, R8_CHIP_ID);
    PrintResetCause();

    /* 1. 溢出中断演示：仅开启中断不复位，溢出后进入中断重装载计数值，当作低频定时器使用 */
    PRINT("\nWWDG interrupt-only mode, waiting %d overflows ...\n", WDOG_FEED_TIMES);
    WWDG_ResetCfg(DISABLE);
    WWDG_ITCfg(ENABLE);
    PFIC_EnableIRQ(WDOG_BAT_IRQn);
    WWDG_SetCounter(0); // 重新装载计数值，从0开始一个完整计数周期
    while(wdog_int_cnt < WDOG_FEED_TIMES)
        ;
    WWDG_ITCfg(DISABLE);
    PFIC_DisableIRQ(WDOG_BAT_IRQn);

    /* 2. 使能看门狗超时复位，主循环喂狗 */
    PRINT("\nWWDG reset mode enabled, feeding %d times ...\n", WDOG_FEED_TIMES);
    WWDG_ResetCfg(ENABLE); // 计数溢出后复位系统
    WWDG_SetCounter(0);    // 重新装载计数值，从0开始一个完整计数周期

    while(feeds < WDOG_FEED_TIMES)
    {
        DelayMs(WDOG_FEED_MS);
        WWDG_SetCounter(0); // 喂狗：重新装载计数值
        feeds++;
        PRINT("  feed %d ok\n", feeds);
    }

    /* 3. 停止喂狗，等待看门狗超时复位（8位计数满约0.56s@60MHz） */
    PRINT("\nStop feeding, watchdog will reset system ...\n");
    while(1)
        ;
}

/*********************************************************************
 * @fn      WDOG_BAT_IRQHandler
 *
 * @brief   看门狗溢出中断函数，仅中断模式下重装载计数值并统计次数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void WDOG_BAT_IRQHandler(void)
{
    wdog_int_cnt++;
    WWDG_SetCounter(0); // 重新装载计数值
    WWDG_ClearFlag();   // 清除看门狗中断标志
}
