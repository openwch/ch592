/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_flash.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_FLASH_H__
#define __CH59x_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   ∂¡»°Flash-ROM
 *
 * @param   StartAddr   - read address
 * @param   Buffer      - read buffer
 * @param   len         - read len
 */
void FLASH_ROM_READ(uint32_t StartAddr, void *Buffer, uint32_t len);

UINT8 UserOptionByteConfig(FunctionalState RESET_EN, FunctionalState BOOT_PIN, FunctionalState UART_NO_KEY_EN,
        uint32_t FLASHProt_Size);

UINT8 UserOptionByteClose_SWD(void);

void UserOptionByte_Active(void);

void GET_UNIQUE_ID(uint8_t *Buffer);
#ifdef __cplusplus
}
#endif

#endif // __CH59x_FLASH_H__
