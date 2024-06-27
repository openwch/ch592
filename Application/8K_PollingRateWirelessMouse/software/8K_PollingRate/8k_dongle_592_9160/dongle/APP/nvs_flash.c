/********************************** (C) COPYRIGHT *******************************
 * File Name          : nvs_flash.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include <rf.h>
#include "HAL.h"
#include "nvs_flash.h"


// 161B
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
nvs_flash_info_t nvs_flash_info;

/*********************************************************************
 * @fn      nvs_flash_padding_check_sum
 *
 * @brief   nvs_flash_padding_check_sum
 *
 * @return  none
 */
static void nvs_flash_padding_check_sum(void)
{
    uint8_t i;
    uint8_t *p;
    p = (uint8_t*)&nvs_flash_info;
    nvs_flash_info.check_sum=0;
    for(i=0; i<sizeof(nvs_flash_info_t)-1; i++)
    {
        nvs_flash_info.check_sum += p[i];
    }
}

/*********************************************************************
 * @fn      nvs_flash_check_check_sum
 *
 * @brief   nvs_flash_check_check_sum
 *
 * @return  none
 */
static uint8_t nvs_flash_check_check_sum(void)
{
    uint8_t i;
    uint8_t *p;
    uint8_t temp=0;
    p = (uint8_t*)&nvs_flash_info;
    for(i=0; i<sizeof(nvs_flash_info_t)-1; i++)
    {
        temp += p[i];
    }
    if(temp!=nvs_flash_info.check_sum)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*********************************************************************
 * @fn      nvs_flash_store
 *
 * @brief   nvs_flash_store
 *
 * @return  none
 */
void nvs_flash_store(void)
{
    nvs_flash_padding_check_sum();
    EEPROM_ERASE(NVS_FLASH_INFO_ADDRESS, EEPROM_PAGE_SIZE);
    EEPROM_WRITE(NVS_FLASH_INFO_ADDRESS, &nvs_flash_info, sizeof(nvs_flash_info_t));
}

/*********************************************************************
 * @fn      nvs_flash_init
 *
 * @brief   nvs_flash_init
 *
 * @return  none
 */
void nvs_flash_init(void)
{
    EEPROM_READ(NVS_FLASH_INFO_ADDRESS, &nvs_flash_info, sizeof(nvs_flash_info_t));
    if((nvs_flash_info.check_val_A != CHECK_VAL_A) || (nvs_flash_info.check_val_B != CHECK_VAL_B) ||
        (nvs_flash_check_check_sum() == FALSE))
    {
        EEPROM_READ(NVS_DFU_FLASH_INFO_ADDRESS, &nvs_flash_info, sizeof(nvs_flash_info_t));
        if((nvs_flash_info.check_val_A != CHECK_VAL_A) || (nvs_flash_info.check_val_B != CHECK_VAL_B) ||
            (nvs_flash_check_check_sum() == FALSE))
        {
            nvs_flash_info.check_val_A = CHECK_VAL_A;
            nvs_flash_info.check_val_B = CHECK_VAL_B;
            tmos_memset(nvs_flash_info.peer_mac, 0, 6);
            nvs_flash_info.capacitance = 5;
        }
        nvs_flash_store();
    }
}

/******************************** endfile @ main ******************************/


