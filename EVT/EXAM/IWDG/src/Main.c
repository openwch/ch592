/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : 独立看门狗IWDG例程：LSI专用时钟驱动，演示周期喂狗与停止喂狗触发系统复位(SDK无IWDG API，直接操作寄存器)
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

/* 预分频32分频(PRB[14:12]=001b)，重装载值0x3FF，LSI按32K计算超时约1s */
#define IWDG_PR_32        0x01
#define IWDG_RLR          1023

/* 喂狗判断阈值：递减到该值前重装载，留有复位余量 */
#define IWDG_FEED_TH      200

/* 喂狗次数：喂够后停止喂狗，演示看门狗复位 */
#define IWDG_FEED_TIMES   5

/* IWDG递减计数器当前值，位于IWDG配置寄存器[27:16] */
#define IWDG_Count_Get()  ((R32_IWDG_CFG >> 16) & 0xFFF)
/* 配置寄存器更新标志，写PR/RLR后需等待其清零 */
#define IWDG_PVU_Get()    ((R32_IWDG_CFG >> 15) & 0x01)
/* 写保护状态，1=禁止操作PR/RLR */
#define IWDG_WR_Protect() ((R32_IWDG_CFG >> 30) & 0x01)

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
 * @fn      IWDG_Enable
 *
 * @brief   独立看门狗使能：启动、解锁写保护、配置预分频与重装载值
 *
 * @param   pr   - 预分频(0=4分频递增至7=512分频)
 *          rlr  - 计数器重装载值，最大0xFFF
 *
 * @return  0成功 1失败
 */
uint8_t IWDG_Enable(uint8_t pr, uint16_t rlr)
{
    uint32_t cfg;

    R32_IWDG_KR = 0xCCCC; // 启动看门狗(需先打开LSI)
    R32_IWDG_KR = 0x5555; // 解除写保护
    if(IWDG_WR_Protect())
        return 1;
    cfg = R32_IWDG_CFG;
    cfg = (cfg & ~(0x7000 | 0xFFF)) | ((uint32_t)(pr & 0x07) << 12) | (rlr & 0xFFF);
    R32_IWDG_CFG = cfg;
    while(IWDG_PVU_Get())
        ; // 等待配置寄存器更新完成
    return 0;
}

/*********************************************************************
 * @fn      IWDG_Feed
 *
 * @brief   喂狗：重装载计数值，从RLR开始递减
 *
 * @return  none
 */
void IWDG_Feed(void)
{
    R32_IWDG_KR = 0xAAAA;
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
    PRINT("IWDG example: feed then timeout reset\n");

    uint8_t feeds = 0;

    LClk32K_Cfg(Clk32K_LSI, ENABLE); // IWDG由专用LSI时钟驱动，必须打开LSI

    /* 配置串口调试 */
    DebugInit();
    PRINT("Time:%s; Start @ChipID=%02X\n", __TIMESTAMP__, R8_CHIP_ID);
    PrintResetCause();

    if(IWDG_Enable(IWDG_PR_32, IWDG_RLR))
    {
        PRINT("IWDG enable failed!\n");
        while(1)
            ;
    }
    IWDG_Feed(); // 装载计数值，从重装载值开始递减

    PRINT("\nIWDG started (PR=32 div, RLR=%d), feeding %d times ...\n", (int)IWDG_RLR, (int)IWDG_FEED_TIMES);

    while(feeds < IWDG_FEED_TIMES)
    {
        if(IWDG_Count_Get() < IWDG_FEED_TH)
        {
            IWDG_Feed(); // 喂狗：重装载计数值
            feeds++;
            PRINT("  feed %d ok\n", feeds);
        }
    }

    PRINT("\nStop feeding, IWDG will reset system ...\n");
    while(1)
        ;
}
