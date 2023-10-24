/********************************** (C) COPYRIGHT *******************************
 * File Name          : Internal_Flash.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : Internal Flash program
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "Internal_Flash.h"
#include "CH59x_common.h"
#include "pdfFile.h"
#include "sw_udisk.h"

#if (STORAGE_MEDIUM == MEDIUM_INTERAL_FLASH)

__attribute__ ((aligned(4))) uint8_t TempBuf[ EEPROM_BLOCK_SIZE ];


void IFlash_Prog_512(uint32_t address,uint32_t *pbuf)
{
    if (address < IFLASH_UDISK_START_ADDR || (address + 511) > IFLASH_UDISK_END_ADDR )
    {
        PRINT("Error Address %x\n",address);
        return;
    }

    FLASH_ROM_READ(address/EEPROM_BLOCK_SIZE*EEPROM_BLOCK_SIZE, TempBuf, EEPROM_BLOCK_SIZE);
    pdf_memcpy(&TempBuf[address%EEPROM_BLOCK_SIZE], pbuf, INTERNAL_FLASH_PAGE_SIZE);
    FLASH_ROM_ERASE(address/EEPROM_BLOCK_SIZE*EEPROM_BLOCK_SIZE, EEPROM_BLOCK_SIZE);
    FLASH_ROM_WRITE(address/EEPROM_BLOCK_SIZE*EEPROM_BLOCK_SIZE, TempBuf, EEPROM_BLOCK_SIZE);
}
#endif
