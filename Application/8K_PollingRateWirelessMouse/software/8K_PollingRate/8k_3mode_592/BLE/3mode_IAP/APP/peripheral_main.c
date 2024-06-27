/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2019/11/05
 * Description        : 判断标志以及搬运代码到APP代码区
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CH59x_common.h"
#include "peripheral.h"
#include "OTA.h"

/* 记录当前的Image */
unsigned char CurrImageFlag = 0xff;

/* flash的数据临时存储 */
__attribute__((aligned(8))) uint8_t block_buf[16];

#define IAP_SAFE_FLAG_2_4G       0x30de5820
#define IAP_SAFE_FLAG_BLE        0x30de5821
#define IAP_SAFE_FLAG_MASK       0x30de5820

#define jump_2_4G    ((void (*)(void))((int *)IMAGE_A_2_4G_START_ADD))
#define jump_BLE    ((void (*)(void))((int *)IMAGE_A_BLE_START_ADD))

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * @fn      SwitchImageFlag
 *
 * @brief   切换dataflash里的ImageFlag
 *
 * @param   new_flag    - 切换的ImageFlag
 *
 * @return  none
 */
void SwitchImageFlag(uint8_t new_flag)
{
    uint16_t i;
    uint32_t ver_flag;

    /* 读取第一块 */
    EEPROM_READ(OTA_DATAFLASH_ADD, (uint32_t *)&block_buf[0], 4);

    /* 擦除第一块 */
    EEPROM_ERASE(OTA_DATAFLASH_ADD, EEPROM_PAGE_SIZE);

    /* 更新Image信息 */
    block_buf[0] = new_flag;

    /* 编程DataFlash */
    EEPROM_WRITE(OTA_DATAFLASH_ADD, (uint32_t *)&block_buf[0], 4);
}

/*********************************************************************
 * @fn      jump_APP
 *
 * @brief   切换APP程序
 *
 * @return  none
 */
void jump_APP(void)
{
    uint8_t *pData;
    uint32_t flash_flag=0;
    pData = (uint8_t *)(IMAGE_B_START_ADD);
    if(CurrImageFlag == IMAGE_IAP_FLAG)
    {
        __attribute__((aligned(8))) uint8_t flash_Data[1024];

        uint8_t i;
        flash_flag = (pData[4]|(pData[5]<<8)|(pData[6]<<16)|(pData[7]<<24));

        if(flash_flag == IAP_SAFE_FLAG_2_4G)
        {
            FLASH_ROM_ERASE(IMAGE_A_2_4G_START_ADD, IMAGE_A_2_4G_SIZE);
            for(i = 0; i < IMAGE_A_2_4G_SIZE / 1024; i++)
            {
                FLASH_ROM_READ(IMAGE_B_START_ADD + (i * 1024), flash_Data, 1024);
                FLASH_ROM_WRITE(IMAGE_A_2_4G_START_ADD + (i * 1024), flash_Data, 1024);
            }
            SwitchImageFlag(IMAGE_A_FLAG);
            // 销毁备份代码
            FLASH_ROM_ERASE(IMAGE_B_START_ADD, IMAGE_A_2_4G_SIZE);
        }
        else if(flash_flag == IAP_SAFE_FLAG_BLE)
        {
            FLASH_ROM_ERASE(IMAGE_A_BLE_START_ADD, IMAGE_A_BLE_SIZE);
            for(i = 0; i < IMAGE_A_BLE_SIZE / 1024; i++)
            {
                FLASH_ROM_READ(IMAGE_B_START_ADD + (i * 1024), flash_Data, 1024);
                FLASH_ROM_WRITE(IMAGE_A_BLE_START_ADD + (i * 1024), flash_Data, 1024);
            }
            SwitchImageFlag(IMAGE_A_FLAG);
            // 销毁备份代码
            FLASH_ROM_ERASE(IMAGE_B_START_ADD, IMAGE_A_2_4G_SIZE);
        }
        else {
            PRINT("flash_flag err %x\n", flash_flag);
            while(1);
        }
    }

    PRINT("ST %x %x\n", KEY_BT_ST,KEY_2_4G_ST);
    DelayMs(10);
    if(!KEY_BT_ST)
    {
        jump_BLE();
    }
    else if(!KEY_2_4G_ST)
    {
        jump_2_4G();
    }
    else
    {
        jump_2_4G();
    }

}

/*********************************************************************
 * @fn      ReadImageFlag
 *
 * @brief   读取当前的程序的Image标志，DataFlash如果为空，就默认是ImageA
 *
 * @return  none
 */
void ReadImageFlag(void)
{
    OTADataFlashInfo_t p_image_flash;

    EEPROM_READ(OTA_DATAFLASH_ADD, &p_image_flash, 4);
    CurrImageFlag = p_image_flash.ImageFlag;

    /* 程序第一次执行，或者没有更新过，以后更新后在擦除DataFlash */
    if((CurrImageFlag != IMAGE_A_FLAG) && (CurrImageFlag != IMAGE_B_FLAG) && (CurrImageFlag != IMAGE_IAP_FLAG))
    {
        CurrImageFlag = IMAGE_A_FLAG;
    }

    PRINT("Image Flag %02x\n", CurrImageFlag);
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
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
#ifdef DEBUG
    GPIOB_SetBits(bTXD2);
    GPIOB_ModeCfg(bTXD2, GPIO_ModeOut_PP_5mA);
    UART2_DefInit();
    UART2_BaudRateCfg( 921600 );
#endif
    GPIOA_ResetBits(KEY_COM);
    GPIOA_ModeCfg(KEY_COM, GPIO_ModeOut_PP_20mA);
    GPIOB_ModeCfg(KEY_BT|KEY_2_4G, GPIO_ModeIN_PU);
    ReadImageFlag();
    jump_APP();
}

/******************************** endfile @ main ******************************/
