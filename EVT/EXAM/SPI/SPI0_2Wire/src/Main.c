/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2026/10/13
 * Description        : SPI0例程：两线模式(8bit)数据收发，主机发送/接收与从机接收/发送，宏切换主从与方向
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

/* 主从模式定义 */
#define SPI_MASTER_MODE       1
#define SPI_SLAVE_MODE        0

/* 主从数据方向定义 */
#define MASTER_SEND_MODE      1 // 主机发送模式，从机接收模式
#define MASTER_RECEIVE_MODE   0 // 主机接收模式，从机发送模式

/* 主从模式选择 */
#define SPI_MODE            SPI_MASTER_MODE      // 切换模式
#define SPI_DIR             MASTER_SEND_MODE     // 切换方向

/* ---- 硬件引脚配置：PA12-SCS PA13-SCK PA14-MOSI PA15-MISO ---- */
#define SPI_CS_PIN      GPIO_Pin_12
#define SPI_SCK_PIN     GPIO_Pin_13
#define SPI_MOSI_PIN    GPIO_Pin_14
#define SPI_MISO_PIN    GPIO_Pin_15

__attribute__((aligned(4))) UINT8 spiBuff[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6};
__attribute__((aligned(4))) UINT8 spiBuffrev[16];

void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

int main(void)
{
    UINT8 i;

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* 配置串口调试 */
    DebugInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);

    /* 芯片版本门禁：低于或等于0x01版本不支持单线SPI */
    if(GET_CHIP_VERSION() <= 0x01)
    {
        PRINT("chip version not support 2wire spi\n");
        while(1);
    }

#if(SPI_MODE == SPI_MASTER_MODE)
    /* ---- 主机模式 ---- */
    PRINT("1.spi 2wire master mode ...\n");
    DelayMs(100);

#if SPI_DIR == MASTER_SEND_MODE
    /* 主机发送模式：主机与从机共用MISO一根数据线 */
    GPIOA_SetBits(SPI_CS_PIN);
    GPIOA_ModeCfg(SPI_CS_PIN | SPI_SCK_PIN | SPI_MISO_PIN, GPIO_ModeOut_PP_5mA);
    SPI_2WIRE_MasterOutputInit();

    // 单字节发送
    GPIOA_ResetBits(SPI_CS_PIN);
    SPI0_MasterSendByte(0x55);
    GPIOA_SetBits(SPI_CS_PIN);
    DelayMs(10);

    // FIFO 连续发送
    GPIOA_ResetBits(SPI_CS_PIN);
    SPI0_MasterTrans(spiBuff, 8);
    GPIOA_SetBits(SPI_CS_PIN);
    DelayMs(10);

    // DMA 连续发送
    GPIOA_ResetBits(SPI_CS_PIN);
    SPI0_MasterDMATrans(spiBuff, 12);
    GPIOA_SetBits(SPI_CS_PIN);
    DelayMs(10);
    PRINT("2wire master send END ...\n");
#else
    /* 主机接收模式：主机与从机共用MISO一根数据线 */
    GPIOA_SetBits(SPI_CS_PIN);
    GPIOA_ModeCfg(SPI_CS_PIN | SPI_SCK_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(SPI_MISO_PIN, GPIO_ModeIN_PU);
    SPI_2WIRE_MasterInputInit();
    GPIOA_ResetBits(SPI_CS_PIN);
    i = SPI0_MasterRecvByte();
    GPIOA_SetBits(SPI_CS_PIN);
    DelayMs(2);
    PRINT("receive %x\n", i);

    GPIOA_ResetBits(SPI_CS_PIN);
    SPI0_MasterRecv(spiBuffrev, 8);
    GPIOA_SetBits(SPI_CS_PIN);
    DelayMs(2);
    PRINT("FIFO recv ");
    for(i = 0; i < 8; i++)
    {
        PRINT(" %x", spiBuffrev[i]);
    }
    PRINT("\n");

    GPIOA_ResetBits(SPI_CS_PIN);
    SPI0_MasterDMARecv(spiBuffrev, 12);
    GPIOA_SetBits(SPI_CS_PIN);
    PRINT("DMA recv ");
    for(i = 0; i < 12; i++)
    {
        PRINT(" %x", spiBuffrev[i]);
    }
    PRINT("\n");
    PRINT("2wire master receive END ...\n");
#endif
#else
    /* ---- 从机模式 ---- */
    PRINT("1.spi 2wire slave mode \n");
    GPIOA_ModeCfg(SPI_CS_PIN | SPI_SCK_PIN | SPI_MISO_PIN, GPIO_ModeIN_PU);

#if SPI_DIR == MASTER_SEND_MODE    // 主机发，从机收模式
    SPI_2WIRE_SlaveInputInit();

    i = SPI0_SlaveRecvByte();
    DelayMs(2);
    PRINT("receive %x\n", i);

    SPI0_SlaveRecv(spiBuffrev, 8);
    DelayMs(2);
    PRINT("FIFO recv ");
    for(i = 0; i < 8; i++)
    {
        PRINT(" %x", spiBuffrev[i]);
    }
    PRINT("\n");

    SPI0_SlaveDMARecv(spiBuffrev, 12);
    PRINT("DMA recv ");
    for(i = 0; i < 12; i++)
    {
        PRINT(" %x", spiBuffrev[i]);
    }
    PRINT("\n");
    PRINT("END ...\n");
#else                              // 主机收，从机发模式
    GPIOA_ModeCfg(SPI_MISO_PIN, GPIO_ModeOut_PP_5mA);
    SPI_2WIRE_SlaveOutputInit();
    SPI0_SlaveSendByte(0xAA);
    SPI0_SlaveTrans(spiBuff, 8);
    SPI0_SlaveDMATrans(spiBuff, 12);
    PRINT("END ...\n");
#endif
#endif

    while(1);
}
