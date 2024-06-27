/********************************** (C) COPYRIGHT *******************************
 * File Name          : PAW3395.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        :
 *
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "peripheral.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*******************************************************************************
 * @fn      spi_set_sc
 *
 * @brief   spi_set_sc
 *
 * @return  None.
 */
void spi_set_sc(uint8_t state)
{
    if (state) {
        GPIOA_ResetBits(SPI_CS);
    } else {
        GPIOA_SetBits(SPI_CS);
    }
}

/*******************************************************************************
 * @fn      spi_write_byte
 *
 * @brief   spi_write_byte
 *
 * @return  None.
 */
__HIGH_CODE
void spi_write_byte(uint8_t data)
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;
    R8_SPI0_BUFFER = data;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_FREE));
}

/*******************************************************************************
 * @fn      spi_read_byte
 *
 * @brief   spi_read_byte
 *
 * @return  None.
 */
__HIGH_CODE
uint8_t spi_read_byte(void)
{
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;
    R8_SPI0_BUFFER = 0xFF; // 启动传输
    while(!(R8_SPI0_INT_FLAG & RB_SPI_FREE));
    return (R8_SPI0_BUFFER);
}

/*******************************************************************************
 * @fn      spi_init
 *
 * @brief   spi_init
 *
 * @return  None.
 */
void spi_init(void)
{
    GPIOA_ModeCfg(SPI_CS | SPI_SCK | SPI_MOSI , GPIO_ModeOut_PP_5mA);
//    GPIOA_SetBits(SPI_MISO);
    GPIOA_ModeCfg(SPI_MISO, GPIO_ModeIN_PU);

    R8_SPI0_CLOCK_DIV = GetSysClock() / 8000000;
//    R8_SPI0_CLOCK_DIV = 250;
    R8_SPI0_CTRL_MOD = RB_SPI_ALL_CLEAR;
    R8_SPI0_CTRL_MOD = RB_SPI_MOSI_OE | RB_SPI_SCK_OE;
    R8_SPI0_CTRL_CFG |= RB_SPI_AUTO_IF;     // 访问BUFFER/FIFO自动清除IF_BYTE_END标志
    R8_SPI0_CTRL_CFG &= ~RB_SPI_DMA_ENABLE; // 不启动DMA方式

    SPI0_DataMode(Mode3_HighBitINFront);
}

/*******************************************************************************
 * @fn      paw3395_write_byte
 *
 * @brief   paw3395_write_byte
 *
 * @return  None.
 */
__HIGH_CODE
void paw3395_write_byte(paw3395_reg_t reg, uint8_t data)
{
    struct paw3395_protocol paw3395_buf_w = {
        .direction = 1,
    };

    paw3395_buf_w.address = reg;
    spi_write_byte(*((uint8_t *)&paw3395_buf_w));
    spi_write_byte(data);
}

/*******************************************************************************
 * @fn      paw3395_write_protect_byte
 *
 * @brief   paw3395_write_protect_byte
 *
 * @return  None.
 */
void paw3395_write_protect_byte(paw3395_reg_t reg, uint8_t data)
{
    paw3395_write_protect_disable();
    paw3395_write_byte(reg, data);
    paw3395_write_protect_enable();
}

/*******************************************************************************
 * @fn      paw3395_read_byte
 *
 * @brief   paw3395_read_byte
 *
 * @return  None.
 */
__HIGH_CODE
uint8_t paw3395_read_byte(paw3395_reg_t reg)
{
    struct paw3395_protocol paw3395_buf_r = {
        .direction = 0,
    };

    paw3395_buf_r.address = reg;
    spi_write_byte(*((uint8_t *)&paw3395_buf_r));
    return spi_read_byte();
}

/*******************************************************************************
 * @fn      paw3395_Power_Up_Init
 *
 * @brief   paw3395_Power_Up_Init
 *
 * @return  None.
 */
static void paw3395_Power_Up_Init( void )
{
    uint8_t read_tmp;
    uint8_t i;
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x40, 0x41 );
    paw3395_write_byte( 0x7F, 0x00 );
    paw3395_write_byte( 0x40, 0x80 );
    paw3395_write_byte( 0x7F, 0x0E );
    paw3395_write_byte( 0x55, 0x0D );
    paw3395_write_byte( 0x56, 0x1B );
    paw3395_write_byte( 0x57, 0xE8 );
    paw3395_write_byte( 0x58, 0xD5 );
    paw3395_write_byte( 0x7F, 0x14 );
    paw3395_write_byte( 0x42, 0xBC );
    paw3395_write_byte( 0x43, 0x74 );
    paw3395_write_byte( 0x4B, 0x20 );
    paw3395_write_byte( 0x4D, 0x00 );
    paw3395_write_byte( 0x53, 0x0E );
    paw3395_write_byte( 0x7F, 0x05 );
    paw3395_write_byte( 0x44, 0x04 );
    paw3395_write_byte( 0x4D, 0x06 );
    paw3395_write_byte( 0x51, 0x40 );
    paw3395_write_byte( 0x53, 0x40 );
    paw3395_write_byte( 0x55, 0xCA );
    paw3395_write_byte( 0x5A, 0xE8 );
    paw3395_write_byte( 0x5B, 0xEA );
    paw3395_write_byte( 0x61, 0x31 );
    paw3395_write_byte( 0x62, 0x64 );
    paw3395_write_byte( 0x6D, 0xB8 );
    paw3395_write_byte( 0x6E, 0x0F );
    paw3395_write_byte( 0x70, 0x02 );
    paw3395_write_byte( 0x4A, 0x2A );
    paw3395_write_byte( 0x60, 0x26 );
    paw3395_write_byte( 0x7F, 0x06 );
    paw3395_write_byte( 0x6D, 0x70 );
    paw3395_write_byte( 0x6E, 0x60 );
    paw3395_write_byte( 0x6F, 0x04 );
    paw3395_write_byte( 0x53, 0x02 );
    paw3395_write_byte( 0x55, 0x11 );

    paw3395_write_byte( 0x7A, 0x01 );
    paw3395_write_byte( 0x7D, 0x51 );
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x41, 0x10 );
    paw3395_write_byte( 0x42, 0x32 );//00
    paw3395_write_byte( 0x43, 0x00 );//04
    paw3395_write_byte( 0x7F, 0x08 );
    paw3395_write_byte( 0x71, 0x4F );
    paw3395_write_byte( 0x7F, 0x09 );
    paw3395_write_byte( 0x62, 0x1F );
    paw3395_write_byte( 0x63, 0x1F );
    paw3395_write_byte( 0x65, 0x03 );
    paw3395_write_byte( 0x66, 0x03 );
    paw3395_write_byte( 0x67, 0x1F );
    paw3395_write_byte( 0x68, 0x1F );
    paw3395_write_byte( 0x69, 0x03 );
    paw3395_write_byte( 0x6A, 0x03 );
    paw3395_write_byte( 0x6C, 0x1F );
    paw3395_write_byte( 0x6D, 0x1F );
    paw3395_write_byte( 0x51, 0x04 );
    paw3395_write_byte( 0x53, 0x20 );
    paw3395_write_byte( 0x54, 0x20 );
    paw3395_write_byte( 0x71, 0x0C );
    paw3395_write_byte( 0x72, 0x07 );
    paw3395_write_byte( 0x73, 0x07 );
    paw3395_write_byte( 0x7F, 0x0A );
    paw3395_write_byte( 0x4A, 0x14 );
    paw3395_write_byte( 0x4C, 0x14 );
    paw3395_write_byte( 0x55, 0x19 );
    paw3395_write_byte( 0x7F, 0x14 );
    paw3395_write_byte( 0x4B, 0x30 );
    paw3395_write_byte( 0x4C, 0x03 );
    paw3395_write_byte( 0x61, 0x0B );
    paw3395_write_byte( 0x62, 0x0A );
    paw3395_write_byte( 0x63, 0x02 );
    paw3395_write_byte( 0x7F, 0x15 );
    paw3395_write_byte( 0x4C, 0x02 );
    paw3395_write_byte( 0x56, 0x02 );
    paw3395_write_byte( 0x41, 0x91 );
    paw3395_write_byte( 0x4D, 0x0A );
    paw3395_write_byte( 0x7F, 0x0C );
    paw3395_write_byte( 0x4A, 0x10 );
    paw3395_write_byte( 0x4B, 0x0C );
    paw3395_write_byte( 0x4C, 0x40 );

    paw3395_write_byte( 0x41, 0x25 );
    paw3395_write_byte( 0x55, 0x18 );
    paw3395_write_byte( 0x56, 0x14 );
    paw3395_write_byte( 0x49, 0x0A );
    paw3395_write_byte( 0x42, 0x00 );//
    paw3395_write_byte( 0x43, 0x2D );//
    paw3395_write_byte( 0x44, 0x0C );
    paw3395_write_byte( 0x54, 0x1A );
    paw3395_write_byte( 0x5A, 0x0D );
    paw3395_write_byte( 0x5F, 0x1E );
    paw3395_write_byte( 0x5B, 0x05 );
    paw3395_write_byte( 0x5E, 0x0F );
    paw3395_write_byte( 0x7F, 0x0D );
    paw3395_write_byte( 0x48, 0xDD );
    paw3395_write_byte( 0x4F, 0x03 );
    paw3395_write_byte( 0x52, 0x49 );
    paw3395_write_byte( 0x51, 0x00 );
    paw3395_write_byte( 0x54, 0x5B );
    paw3395_write_byte( 0x53, 0x00 );
    paw3395_write_byte( 0x56, 0x64 );
    paw3395_write_byte( 0x55, 0x00 );
    paw3395_write_byte( 0x58, 0xA5 );
    paw3395_write_byte( 0x57, 0x02 );
    paw3395_write_byte( 0x5A, 0x29 );
    paw3395_write_byte( 0x5B, 0x47 );
    paw3395_write_byte( 0x5C, 0x81 );
    paw3395_write_byte( 0x5D, 0x40 );
    paw3395_write_byte( 0x71, 0xDC );
    paw3395_write_byte( 0x70, 0x07 );
    paw3395_write_byte( 0x73, 0x00 );
    paw3395_write_byte( 0x72, 0x08 );
    paw3395_write_byte( 0x75, 0xDC );
    paw3395_write_byte( 0x74, 0x07 );
    paw3395_write_byte( 0x77, 0x00 );
    paw3395_write_byte( 0x76, 0x08 );
    paw3395_write_byte( 0x7F, 0x10 );
    paw3395_write_byte( 0x4C, 0xD0 );
    paw3395_write_byte( 0x7F, 0x00 );
    paw3395_write_byte( 0x4F, 0x63 );
    paw3395_write_byte( 0x4E, 0x00 );
    paw3395_write_byte( 0x52, 0x63 );
    paw3395_write_byte( 0x51, 0x00 );
    paw3395_write_byte( 0x54, 0x54 );

    paw3395_write_byte( 0x5A, 0x10 );
    paw3395_write_byte( 0x77, 0x4F );
    paw3395_write_byte( 0x47, 0x01 );
    paw3395_write_byte( 0x5B, 0x40 );
    paw3395_write_byte( 0x64, 0x60 );
    paw3395_write_byte( 0x65, 0x06 );
    paw3395_write_byte( 0x66, 0x13 );
    paw3395_write_byte( 0x67, 0x0F );
    paw3395_write_byte( 0x78, 0x01 );
    paw3395_write_byte( 0x79, 0x9C );
    paw3395_write_byte( 0x40, 0x00 );
    paw3395_write_byte( 0x55, 0x02 );
    paw3395_write_byte( 0x23, 0x70 );
    paw3395_write_byte( 0x22, 0x01 );
//Wait for 1ms
    DelayMs( 1 );
    for( i = 0; i < 60; i++ )
    {
        read_tmp = paw3395_read_byte( 0x6C );
        if(read_tmp == 0x80)
            break;
        DelayMs( 1 );
    }
    if( i == 60 )
    {
        paw3395_write_byte( 0x7F, 0x14 );
        paw3395_write_byte( 0x6C, 0x00 );
        paw3395_write_byte( 0x7F, 0x00 );
    }
    paw3395_write_byte( 0x22, 0x00 );
    paw3395_write_byte( 0x55, 0x00 );
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x40, 0x40 );
    paw3395_write_byte( 0x7F, 0x00 );
}

/*******************************************************************************
 * @fn      paw3395_corded_Mode
 *
 * @brief   paw3395_corded_Mode
 *
 * @return  None.
 */
static void paw3395_8K_Mode( void )
{
    GPIOA_ResetBits(SPI_CS);

    paw3395_write_byte( 0x7F, 0x05 );
    paw3395_write_byte( 0x51, 0x40 );
    paw3395_write_byte( 0x53, 0x40 );
    paw3395_write_byte( 0x61, 0x31 );
    paw3395_write_byte( 0x6E, 0x0F );
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x42, 0x2F );
    paw3395_write_byte( 0x43, 0x00 );
    paw3395_write_byte( 0x7F, 0x0D );
    paw3395_write_byte( 0x51  , 0x12 );
    paw3395_write_byte( 0x52  , 0xDB );
    paw3395_write_byte( 0x53  , 0x12 );
    paw3395_write_byte( 0x54  , 0xDC );
    paw3395_write_byte( 0x55  , 0x12 );
    paw3395_write_byte( 0x56  , 0xEA );
    paw3395_write_byte( 0x57  , 0x15 );
    paw3395_write_byte( 0x58  , 0x2D );
    paw3395_write_byte( 0x7F  , 0x00 );
    paw3395_write_byte( 0x54  , 0x55 );
    paw3395_write_byte( 0x40  , 0x83 );
    GPIOA_SetBits(SPI_CS);
}


/*******************************************************************************
 * @fn      paw3395_other_Init
 *
 * @brief   paw3395_other_Init
 *
 * @return  None.
 */
static void paw3395_other_Init( void )
{
//    uint8_t read_tmp;
//    read_tmp = paw3395_read_byte( 0x40 );
//    if( read_tmp != 0 )
//    {
//        PRINT("1 0x40: %#x\n", read_tmp);
//        return;
//    }
//
//    paw3395_write_byte( 0x7F, 0x05 );
//    paw3395_write_byte( 0x51, 0x40 );
//    paw3395_write_byte( 0x53, 0x40 );
//    paw3395_write_byte( 0x61, 0x31 );
//    paw3395_write_byte( 0x6E, 0x0F );
//
//    paw3395_write_byte( 0x7F, 0x07 );
//    paw3395_write_byte( 0x51, 0x12 );
//    paw3395_write_byte( 0x52, 0xDB );
//    paw3395_write_byte( 0x53, 0x12 );
//    paw3395_write_byte( 0x54, 0xDC );
//    paw3395_write_byte( 0x55, 0x12 );
//    paw3395_write_byte( 0x56, 0xEA );
//    paw3395_write_byte( 0x57, 0x15 );
//    paw3395_write_byte( 0x58, 0x2D );
//    paw3395_write_byte( 0x7F, 0x00 );
//
//    paw3395_write_byte( 0x54, 0x55 );
//    paw3395_write_byte( 0x40, 0x83 );
//    paw3395_write_byte( 0x5A, 0x10 );
//    paw3395_write_byte( 0x7F, 0x0D );
//    paw3395_write_byte( 0x48, 0xDD );
//    paw3395_write_byte( 0x7F, 0x00 );
//    paw3395_write_byte( 0x56, 0x0D );
//
//    paw3395_read_byte(PAW_MOTION_STATUS);
//    paw3395_read_byte(PAW_DELTA_X_L);
//    paw3395_read_byte(PAW_DELTA_X_H);
//    paw3395_read_byte(PAW_DELTA_Y_L);
//    paw3395_read_byte(PAW_DELTA_Y_H);
//
//    paw3395_write_byte( 0x7F, 0x0C );
//    paw3395_write_byte( 0x4E, 0x0A );
//    paw3395_write_byte( 0x7F, 0x00 );
//    paw3395_write_byte( 0x7F, 0x0C );
//    paw3395_write_byte( 0x61, 0x22 );
//    paw3395_write_byte( 0x62, 0x41 );
//    paw3395_write_byte( 0x7F, 0x00 );
//    paw3395_write_byte( 0x48, 0x3F );
//    paw3395_write_byte( 0x49, 0x00 );
//    paw3395_write_byte( 0x4A, 0x3F );
//    paw3395_write_byte( 0x4B, 0x00 );
//    paw3395_write_byte( 0x47, 0x01 );
////    paw3395_read_byte(PAW_MOTION_BURST);
////    spi_read_byte();
////    spi_read_byte();
////    spi_read_byte();
////    spi_read_byte();
////    spi_read_byte();
////    spi_read_byte();
//    read_tmp = paw3395_read_byte( 0x40 );
//    if( read_tmp != 0x83 )
//    {
//        PRINT("2 0x40: %#x\n", read_tmp);
//        return;
//    }
   if(work_mode == MODE_2_4G)
   {
       paw3395_8K_Mode();
       PRINT("8k Mode\n");
   }
   else
   {
       paw3395_lowpower_Mode();
       PRINT("LowPower Mode\n");
   }

//    paw3395_4K_Mode();
//    paw3395_gaming_Mode();

//    paw3395_office_Mode();
}

/*******************************************************************************
 * @fn      paw3395_corded_Mode
 *
 * @brief   paw3395_corded_Mode
 *
 * @return  None.
 */
static void paw3395_4K_Mode( void )
{
    GPIOA_ResetBits(SPI_CS);

    paw3395_write_byte( 0x7F, 0x05 );
    paw3395_write_byte( 0x51, 0x40 );
    paw3395_write_byte( 0x53, 0x40 );
    paw3395_write_byte( 0x61, 0x31 );
    paw3395_write_byte( 0x6E, 0x0F );
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x42, 0x32 );
    paw3395_write_byte( 0x43, 0x00 );
    paw3395_write_byte( 0x7F, 0x0D );
    paw3395_write_byte( 0x51  , 0x00 );
    paw3395_write_byte( 0x52  , 0x49 );
    paw3395_write_byte( 0x53  , 0x00 );
    paw3395_write_byte( 0x54  , 0x5B );
    paw3395_write_byte( 0x55  , 0x00 );
    paw3395_write_byte( 0x56  , 0x64 );
    paw3395_write_byte( 0x57  , 0x02 );
    paw3395_write_byte( 0x58  , 0xA5 );
    paw3395_write_byte( 0x7F  , 0x00 );
    paw3395_write_byte( 0x54  , 0x54 );
    paw3395_write_byte( 0x78  , 0x01 );
    paw3395_write_byte( 0x79  , 0x9C );
    paw3395_write_byte( 0x40  , 0x03 );
    GPIOA_SetBits(SPI_CS);
}

/*******************************************************************************
 * @fn      paw3395_gaming_Mode
 *
 * @brief   paw3395_gaming_Mode
 *
 * @return  None.
 */
static void paw3395_gaming_Mode( void )
{
    GPIOA_ResetBits(SPI_CS);

    paw3395_write_byte( 0x7F, 0x05 );
    paw3395_write_byte( 0x51, 0x40 );
    paw3395_write_byte( 0x53, 0x40 );
    paw3395_write_byte( 0x61, 0x31 );
    paw3395_write_byte( 0x6E, 0x0F );
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x42, 0x32 );
    paw3395_write_byte( 0x43, 0x00 );
    paw3395_write_byte( 0x7F, 0x0D );
    paw3395_write_byte( 0x51  , 0x00 );
    paw3395_write_byte( 0x52  , 0x49 );
    paw3395_write_byte( 0x53  , 0x00 );
    paw3395_write_byte( 0x54  , 0x5B );
    paw3395_write_byte( 0x55  , 0x00 );
    paw3395_write_byte( 0x56  , 0x64 );
    paw3395_write_byte( 0x57  , 0x02 );
    paw3395_write_byte( 0x58  , 0xA5 );
    paw3395_write_byte( 0x7F  , 0x00 );
    paw3395_write_byte( 0x54  , 0x54 );
    paw3395_write_byte( 0x78  , 0x01 );
    paw3395_write_byte( 0x79  , 0x9C );
    paw3395_write_byte( 0x40  , paw3395_read_byte(0x40)&0xFC );
    GPIOA_SetBits(SPI_CS);
}

/*******************************************************************************
 * @fn      paw3395_other_Init
 *
 * @brief   paw3395_other_Init
 *
 * @return  None.
 */
static void paw3395_lowpower_Mode( void )
{
    GPIOA_ResetBits(SPI_CS);
    paw3395_write_byte( 0x7F, 0x05 );
    paw3395_write_byte( 0x51, 0x40 );
    paw3395_write_byte( 0x53, 0x40 );
    paw3395_write_byte( 0x61, 0x3B );
    paw3395_write_byte( 0x6E, 0x1F );
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x42, 0x32 );
    paw3395_write_byte( 0x43, 0x00 );
    paw3395_write_byte( 0x7F, 0x0D );
    paw3395_write_byte( 0x51  , 0x00 );
    paw3395_write_byte( 0x52  , 0x49 );
    paw3395_write_byte( 0x53  , 0x00 );
    paw3395_write_byte( 0x54  , 0x5B );
    paw3395_write_byte( 0x55  , 0x00 );
    paw3395_write_byte( 0x56  , 0x64 );
    paw3395_write_byte( 0x57  , 0x02 );
    paw3395_write_byte( 0x58  , 0xA5 );
    paw3395_write_byte( 0x7F  , 0x00 );
    paw3395_write_byte( 0x54  , 0x54 );
    paw3395_write_byte( 0x78  , 0x01 );
    paw3395_write_byte( 0x79  , 0x9C );
    paw3395_write_byte( 0x40  , (paw3395_read_byte(0x40)&0xFC)|0x01 );
    GPIOA_SetBits(SPI_CS);
}

/*******************************************************************************
 * @fn      paw3395_office_Mode
 *
 * @brief   paw3395_office_Mode
 *
 * @return  None.
 */
static void paw3395_office_Mode( void )
{
    GPIOA_ResetBits(SPI_CS);
    paw3395_write_byte( 0x7F, 0x05 );
    paw3395_write_byte( 0x51, 0x28 );
    paw3395_write_byte( 0x53, 0x30 );
    paw3395_write_byte( 0x61, 0x3B );
    paw3395_write_byte( 0x6E, 0x1F );
    paw3395_write_byte( 0x7F, 0x07 );
    paw3395_write_byte( 0x42, 0x32 );
    paw3395_write_byte( 0x43, 0x00 );
    paw3395_write_byte( 0x7F, 0x0D );
    paw3395_write_byte( 0x51  , 0x00 );
    paw3395_write_byte( 0x52  , 0x49 );
    paw3395_write_byte( 0x53  , 0x00 );
    paw3395_write_byte( 0x54  , 0x5B );
    paw3395_write_byte( 0x55  , 0x00 );
    paw3395_write_byte( 0x56  , 0x64 );
    paw3395_write_byte( 0x57  , 0x02 );
    paw3395_write_byte( 0x58  , 0xA5 );
    paw3395_write_byte( 0x7F  , 0x00 );
    paw3395_write_byte( 0x54  , 0x54 );
    paw3395_write_byte( 0x78  , 0x0A );
    paw3395_write_byte( 0x79  , 0x0F );
    paw3395_write_byte( 0x40  , (paw3395_read_byte(0x40)&0xFC)|0x02 );
    GPIOA_SetBits(SPI_CS);
}

/*******************************************************************************
 * @fn      paw3395_init
 *
 * @brief   paw3395_init
 *
 * @return  None.
 */
void paw3395_init(void)
{

    GPIOA_SetBits(SPI_CS);
    spi_init();

    /* Sensor need time to stabilize */
    DelayMs(500);
    GPIOA_ResetBits(SPI_CS);
    paw3395_write_byte(0x3B, 0xB6);
    GPIOA_SetBits(SPI_CS);
    DelayMs(300);

    GPIOA_ResetBits(SPI_CS);
    DelayUs(40);
    GPIOA_SetBits(SPI_CS);
    DelayUs(40);
    GPIOA_ResetBits(SPI_CS);
    paw3395_write_byte(PAW_POWER_UP_RESET, 0x5A);

    GPIOA_SetBits(SPI_CS);
    DelayMs(50);

//    spi_set_sc(0);
    GPIOA_ResetBits(SPI_CS);
    paw3395_Power_Up_Init( );

    paw3395_other_Init();
//    GPIOA_SetBits(SPI_CS);
//    DelayMs(1);
//
//    GPIOA_ResetBits(SPI_CS);
//    paw3395_read_byte(PAW_MOTION_STATUS);
//    DelayUs(1);
//    paw3395_read_byte(PAW_DELTA_X_L);
//    DelayUs(1);
//    paw3395_read_byte(PAW_DELTA_X_H);
//    DelayUs(1);
//    paw3395_read_byte(PAW_DELTA_Y_L);
//    DelayUs(1);
//    paw3395_read_byte(PAW_DELTA_Y_H);
//    DelayUs(1);
    PRINT("pid: %#x\n", paw3395_read_byte(PAW_PID1));

    GPIOA_SetBits(SPI_CS);
    DelayUs(1);
}

/******************************** endfile @rf ******************************/
