/********************************** (C) COPYRIGHT *******************************
 * File Name          : SPI_FLAH.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2022/11/24
 * Description        : SPI FLASHChip operation file
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/


/******************************************************************************/
/* Header Files */
#include "spi_flash.h"
#include "sw_udisk.h"
#include "peripheral.h"


//#define CONFIG_SPI_FLASH_DEBUG
#ifdef CONFIG_SPI_FLASH_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

/******************************************************************************/
volatile uint8_t   Flash_Type = 0x00;                                           /* FLASH chip: 0: W25XXXseries */
volatile uint32_t  Flash_ID = 0x00;                                             /* FLASH ID */
volatile uint32_t  Flash_Sector_Count = 0x00;                                   /* FLASH sector number */
volatile uint16_t  Flash_Sector_Size = 0x00;                                    /* FLASH sector size */

/*******************************************************************************
* Function Name  : FLASH_Port_Init
* Description    : FLASH chip operation related pins and hardware initialization
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_Port_Init( void )
{
    GPIOA_SetBits(GPIO_Pin_12);
    GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_Floating);

    R8_SPI0_CLOCK_DIV = 4; // System clock divided by 4
    R8_SPI0_CTRL_MOD = RB_SPI_ALL_CLEAR;
    R8_SPI0_CTRL_MOD = RB_SPI_MOSI_OE | RB_SPI_SCK_OE; //Mode0
    R8_SPI0_CTRL_CFG |= RB_SPI_AUTO_IF;     // Automatically clear IF_BYTE_END flag when accessing BUFFER/FIFO
    R8_SPI0_CTRL_CFG &= ~RB_SPI_DMA_ENABLE; // DMA not enabled
}

/*********************************************************************
 * @fn      SPI_ReadWriteByte
 *
 * @brief   SPIx read or write one byte.
 *
 * @param   TxData - write one byte data.
 *
 * @return  Read one byte data.
 */
uint8_t SPI_ReadWriteByte(uint8_t TxData)
{
    uint8_t i = 0;

    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;
    R8_SPI0_BUFFER = TxData;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_FREE))
    {
        i++;
        if(i > 200)
            return 0;
    }
    return (R8_SPI0_BUFFER);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : SPI send a byte
* Input          : byte: byte to send
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t SPI_FLASH_SendByte( uint8_t byte )
{
    return SPI_ReadWriteByte( byte );
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : SPI receive a byte
* Input          : None
* Output         : None
* Return         : byte received
*******************************************************************************/
uint8_t SPI_FLASH_ReadByte( void )
{
    return SPI_ReadWriteByte( 0xFF );
}

/*******************************************************************************
* Function Name  : FLASH_ReadID
* Description    : Read FLASH ID
* Input          : None
* Output         : None
* Return         : chip id
*******************************************************************************/
uint32_t FLASH_ReadID( void )
{
    uint32_t dat;
    PIN_FLASH_CS_LOW( );
    SPI_FLASH_SendByte( CMD_FLASH_JEDEC_ID );
    dat = ( uint32_t )SPI_FLASH_SendByte( DEF_DUMMY_BYTE ) << 16;
    dat |= ( uint32_t )SPI_FLASH_SendByte( DEF_DUMMY_BYTE ) << 8;
    dat |= SPI_FLASH_SendByte( DEF_DUMMY_BYTE );
    PIN_FLASH_CS_HIGH( );
    return( dat );
}

/*******************************************************************************
* Function Name  : FLASH_WriteEnable
* Description    : FLASH Write Enable
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_WriteEnable( void )
{
    PIN_FLASH_CS_LOW( );
    SPI_FLASH_SendByte( CMD_FLASH_WREN );
    PIN_FLASH_CS_HIGH( );
}

/*******************************************************************************
* Function Name  : FLASH_WriteDisable
* Description    : FLASH Write Disable
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_WriteDisable( void )
{
    PIN_FLASH_CS_LOW( );
    SPI_FLASH_SendByte( CMD_FLASH_WRDI );
    PIN_FLASH_CS_HIGH( );
}

/*******************************************************************************
* Function Name  : FLASH_ReadStatusReg
* Description    : FLASH Read Status
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint8_t FLASH_ReadStatusReg( void )
{
    uint8_t  status;

    PIN_FLASH_CS_LOW( );
    SPI_FLASH_SendByte( CMD_FLASH_RDSR );
    status = SPI_FLASH_ReadByte( );
    PIN_FLASH_CS_HIGH( );
    return( status );
}

/*******************************************************************************
* Function Name  : FLASH_Erase_Sector
* Description    : FLASH Erase Sector
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/ 
void FLASH_Erase_Sector( uint32_t address )
{
    uint8_t  temp;
    FLASH_WriteEnable( );
    PIN_FLASH_CS_LOW( );
    SPI_FLASH_SendByte( CMD_FLASH_SECTOR_ERASE );
    SPI_FLASH_SendByte( (uint8_t)( address >> 16 ) );
    SPI_FLASH_SendByte( (uint8_t)( address >> 8 ) );
    SPI_FLASH_SendByte( (uint8_t)address );
    PIN_FLASH_CS_HIGH( );
    do
    {
        temp = FLASH_ReadStatusReg( );    
    }while( temp & 0x01 );
}

/*******************************************************************************
* Function Name  : FLASH_RD_Block_Start
* Description    : FLASH start block read
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/  
void FLASH_RD_Block_Start( uint32_t address )
{
    PIN_FLASH_CS_LOW( );
    SPI_FLASH_SendByte( CMD_FLASH_READ );
    SPI_FLASH_SendByte( (uint8_t)( address >> 16 ) );
    SPI_FLASH_SendByte( (uint8_t)( address >> 8 ) );
    SPI_FLASH_SendByte( (uint8_t)address );
}

/*******************************************************************************
* Function Name  : FLASH_RD_Block
* Description    : FLASH read block
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/  
void FLASH_RD_Block( uint8_t *pbuf, uint32_t len )
{
    while( len-- )                                                             
    {
        *pbuf++ = SPI_FLASH_ReadByte( );
    }
}

/*******************************************************************************
* Function Name  : FLASH_RD_Block_End
* Description    : FLASH end block read
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_RD_Block_End( void )
{
    PIN_FLASH_CS_HIGH( );
}

/*******************************************************************************
* Function Name  : W25XXX_WR_Page
* Description    : Flash page program
* Input          : address
*                  len
*                  *pbuf
* Output         : None
* Return         : None
*******************************************************************************/
void W25XXX_WR_Page( uint8_t *pbuf, uint32_t address, uint32_t len )
{
    uint8_t  temp;
    FLASH_WriteEnable( );
    PIN_FLASH_CS_LOW( );
    SPI_FLASH_SendByte( CMD_FLASH_BYTE_PROG );
    SPI_FLASH_SendByte( (uint8_t)( address >> 16 ) );
    SPI_FLASH_SendByte( (uint8_t)( address >> 8 ) );
    SPI_FLASH_SendByte( (uint8_t)address );
    if( len > SPI_FLASH_PerWritePageSize )
    {
        len = SPI_FLASH_PerWritePageSize;
    }
    while( len-- )
    {
        SPI_FLASH_SendByte( *pbuf++ );
    }
    PIN_FLASH_CS_HIGH( );
    do
    {
        temp = FLASH_ReadStatusReg( );    
    }while( temp & 0x01 );
}

//__attribute__ ((aligned(4))) uint8_t TempBuffer[ EEPROM_BLOCK_SIZE ];
void W25XXX_WR_512B( uint8_t *pbuf, uint32_t address, uint32_t len )
{
    __attribute__ ((aligned(4))) uint8_t TempBuffer[ EEPROM_BLOCK_SIZE ];

    FLASH_RD_Block_Start( address/EEPROM_BLOCK_SIZE*EEPROM_BLOCK_SIZE );
    FLASH_RD_Block(TempBuffer, 4096);
    FLASH_RD_Block_End();

    pdf_memcpy(&TempBuffer[address%EEPROM_BLOCK_SIZE], pbuf, 512);
    FLASH_Erase_Sector(address/EEPROM_BLOCK_SIZE*EEPROM_BLOCK_SIZE);
//    FLASH_ROM_WRITE(address/EEPROM_BLOCK_SIZE*EEPROM_BLOCK_SIZE, TempBuf, EEPROM_BLOCK_SIZE);

    for(uint8_t i=0; i<EEPROM_BLOCK_SIZE/256; ++i)
    {
        W25XXX_WR_Page(&TempBuffer[256*i], address/EEPROM_BLOCK_SIZE*EEPROM_BLOCK_SIZE+256*i, 256);
    }
}

/*******************************************************************************
* Function Name  : W25XXX_WR_Block
* Description    : W25XXX block write 
* Input          : address
*                  len
*                  *pbuf
* Output         : None
* Return         : None
*******************************************************************************/
void W25XXX_WR_Block( uint8_t *pbuf, uint32_t address, uint32_t len )
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    Addr = address % SPI_FLASH_PageSize;
    count = SPI_FLASH_PageSize - Addr;
    NumOfPage =  len / SPI_FLASH_PageSize;
    NumOfSingle = len % SPI_FLASH_PageSize;

    if( Addr == 0 )
    {
        if( NumOfPage == 0 )
        {
            W25XXX_WR_Page( pbuf, address, len );
        }
        else
        {
            while( NumOfPage-- )
            {
                W25XXX_WR_Page( pbuf, address, SPI_FLASH_PageSize );
                address +=  SPI_FLASH_PageSize;
                pbuf += SPI_FLASH_PageSize;
            }
            W25XXX_WR_Page( pbuf, address, NumOfSingle );
        }
    }
    else
    {
        if( NumOfPage == 0 )
        {
            if( NumOfSingle > count )
            {
                temp = NumOfSingle - count;
                W25XXX_WR_Page( pbuf, address, count );
                address +=  count;
                pbuf += count;
                W25XXX_WR_Page( pbuf, address, temp );
            }
            else
            {
                W25XXX_WR_Page( pbuf, address, len );
            }
        }
        else
        {
            len -= count;
            NumOfPage =  len / SPI_FLASH_PageSize;
            NumOfSingle = len % SPI_FLASH_PageSize;
            W25XXX_WR_Page( pbuf, address, count );
            address +=  count;
            pbuf += count;
            while( NumOfPage-- )
            {
                W25XXX_WR_Page( pbuf, address, SPI_FLASH_PageSize );
                address += SPI_FLASH_PageSize;
                pbuf += SPI_FLASH_PageSize;
            }
            if( NumOfSingle != 0 )
            {
                W25XXX_WR_Page( pbuf, address, NumOfSingle );
            }
        }
    }
}



/*******************************************************************************
* Function Name  : FLASH_IC_Check
* Description    : check flash type
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IC_Check( void )
{
    uint32_t count;

    /* Read FLASH ID */    
    Flash_ID = FLASH_ReadID( );
    LOG_INFO("Flash_ID: %08x\n",(uint32_t)Flash_ID);

    Flash_Type = 0x00;                                                                
    Flash_Sector_Count = 0x00;
    Flash_Sector_Size = 0x00;

    switch( Flash_ID )
    {
        /* W25XXX */
        case W25X10_FLASH_ID:                                                   /* 0xEF3011-----1M bit */
            count = 1;
            break;

        case W25X20_FLASH_ID:                                                   /* 0xEF3012-----2M bit */
            count = 2;
            break;

        case W25X40_FLASH_ID:                                                   /* 0xEF3013-----4M bit */
            count = 4;
            break;

        case W25X80_FLASH_ID:                                                   /* 0xEF4014-----8M bit */
            count = 8;
            break;

        case W25Q16_FLASH_ID1:                                                  /* 0xEF3015-----16M bit */
        case W25Q16_FLASH_ID2:                                                  /* 0xEF4015-----16M bit */
            count = 16;
            break;

        case W25Q32_FLASH_ID1:                                                  /* 0xEF4016-----32M bit */
        case W25Q32_FLASH_ID2:                                                  /* 0xEF6016-----32M bit */
            count = 32;
            break;

        case W25Q64_FLASH_ID1:                                                  /* 0xEF4017-----64M bit */
        case W25Q64_FLASH_ID2:                                                  /* 0xEF6017-----64M bit */
            count = 64;
            break;

        case W25Q128_FLASH_ID1:                                                 /* 0xEF4018-----128M bit */
        case W25Q128_FLASH_ID2:                                                 /* 0xEF6018-----128M bit */
            count = 128;
            break;

        case W25Q256_FLASH_ID1:                                                 /* 0xEF4019-----256M bit */
        case W25Q256_FLASH_ID2:                                                 /* 0xEF6019-----256M bit */
            count = 256;
            break;
        default:
            if( ( Flash_ID != 0xFFFFFFFF ) && ( Flash_ID != 0x00000000 ) )
            {
                count = 16;
            }
            else
            {
                count = 0x00;
            }
            break;
    }
    count = ( (uint32_t)count * 1024 ) * ( (uint32_t)1024 / 8 );

    if( count )
    {
        Flash_Sector_Count = count / DEF_UDISK_SECTOR_SIZE;
        Flash_Sector_Size = DEF_UDISK_SECTOR_SIZE;
    }
    else
    {
        LOG_INFO("External Flash not connected\r\n");
        while(1);
    }
}

