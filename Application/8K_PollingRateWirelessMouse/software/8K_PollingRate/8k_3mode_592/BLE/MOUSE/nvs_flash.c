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
#include "peripheral.h"
#include "nvs_flash.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */


nvs_flash_info_t nvs_flash_info;

static const uint8_t nvs_flash_usb_prod_info[] = "IA";
// Device name
static CONST uint8_t nvs_flash_device_name[] = "Bluetooth mouse";

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
    PFIC_DisableIRQ(RTC_IRQn);
    mDelaymS(1);
    EEPROM_ERASE(NVS_FLASH_INFO_ADDRESS, EEPROM_PAGE_SIZE);
    EEPROM_WRITE(NVS_FLASH_INFO_ADDRESS, &nvs_flash_info, sizeof(nvs_flash_info_t));
    PFIC_EnableIRQ(RTC_IRQn);
}

/*********************************************************************
 * @fn      nvs_flash_init
 *
 * @brief   初始化其他功能之前调用，确保使用的是flash中的数据。
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
            nvs_flash_info.led_onoff = TRUE;
            nvs_flash_info.led_mode = LED_MODE_ON;
            nvs_flash_info.dpi = DPI_5000;
            nvs_flash_info.ble_idx = BLE_INDEX_1;
            nvs_flash_info.ble_bond_flag = 0;
            nvs_flash_info.ble_mac_flag = 0;
            nvs_flash_info.ble_name_len = sizeof(nvs_flash_device_name)-1;
            tmos_memcpy(nvs_flash_info.ble_name_data, nvs_flash_device_name, nvs_flash_info.ble_name_len);
            nvs_flash_info.led_light = LED_LIGHT_100;
            nvs_flash_info.work_mode = SYS_WIN;
            nvs_flash_info.rf_device_id = RF_ROLE_ID_INVALD;
            tmos_memset(nvs_flash_info.peer_mac, 0, 6);
            nvs_flash_info.usb_vid_pid[0] = USB_VID&0xFF;
            nvs_flash_info.usb_vid_pid[1] = (USB_VID>>8)&0xFF;
            nvs_flash_info.usb_vid_pid[2] = USB_PID&0xFF;
            nvs_flash_info.usb_vid_pid[3] = (USB_PID>>8)&0xFF;
            nvs_flash_info.capacitance = 5;
            nvs_flash_info.usb_prod_info_len = sizeof(nvs_flash_usb_prod_info);
            tmos_memcpy(nvs_flash_info.usb_prod_info, nvs_flash_usb_prod_info, nvs_flash_info.usb_prod_info_len);
        }
        else
        {
           PRINT("read DFU flash success!\n");
        }
        nvs_flash_store();
//        printf("\n");
//        for(uint8_t i = 0; i < sizeof(nvs_flash_info_t); i++)
//        {
//            printf("%02x ",((uint8_t *)&nvs_flash_info)[i]);
//        }
//        printf("\n");
//        printf("\n");
    }
    PRINT("peer mac ");
    for(uint8_t i = 0; i < 6; i++)
    {
        PRINT("%x ",nvs_flash_info.peer_mac[i]);
    }
    PRINT("\n");
}

/******************************** endfile @ main ******************************/
