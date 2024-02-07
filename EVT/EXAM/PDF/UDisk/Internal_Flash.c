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

/******************************************************************************/
/* Header Files */
#include "Internal_Flash.h"
#include "CH59x_common.h"
#include "pdfFile.h"

__attribute__ ((aligned(4))) uint8_t tempbuf[ 4096 ];

/*********************************************************************
 * @fn      IFlash_Prog_512
 *
 * @brief   This function is used to write 512 bytes of data to the specified address's flash memory
 *
 * @param   address - The starting address of the Flash memory to be programmed
 *          pbuf - Pointer to the data buffer used to store data to be programmed into Flash memory
 *
 * @return  none
 */
void IFlash_Prog_512(uint32_t address,uint32_t *pbuf)
{

    if (address < IFLASH_UDISK_START_ADDR || (address + 511) > IFLASH_UDISK_END_ADDR )
    {
        printf("Error Address %x\n",address);
        return;
    }
    FLASH_ROM_READ(address/4096*4096, tempbuf, 4096);                    // The starting address of the read flash block is address/4096 * 4096
    pdf_memcpy(&tempbuf[address%4096], pbuf, INTERNAL_FLASH_PAGE_SIZE);  // Copy the data pointed to by pbuf to the specified position in the tempbuf array
    FLASH_ROM_ERASE(address/4096*4096, 4096);                            // The starting address of the erased flash block is address/4096 * 4096, with a size of 4096 bytes
    FLASH_ROM_WRITE(address/4096*4096, tempbuf, 4096);                   // Write the data in the tempbuf array to the erased flash block.
                                                                         // The starting address of the written flash block is address/4096 * 4096, with a size of 4096 bytes.
}

