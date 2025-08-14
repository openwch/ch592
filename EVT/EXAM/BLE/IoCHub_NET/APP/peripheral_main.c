/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : ����ӻ�Ӧ��������������ϵͳ��ʼ��
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* ͷ�ļ����� */
#include "CONFIG.h"
#include "HAL.h"
#include "peripheral.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t  MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif



volatile uint8_t switch_sta = FALSE;

/*********************************************************************
 * @fn      Key_Scan(���е͹����������������TMOS��ѯ������ִ��)
 *
 * @brief   ����ɨ��-mode 0-��֧������ 1-֧������
 * @define FILTER_TIMES    mode 0-�������� mode 1-���������������
 * @return  Key_Value
 */
uint8_t Key_Scan(uint8_t mode)
{
    static uint16_t flag = FILTER_TIMES;
    static uint8_t key_value_reg;
    uint8_t key_value = KEY_NONE;
    __IO uint32_t IO_KEY = GPIOB_ReadPortPin(KEY_SWITCH_PIN);

    if(mode == 1)
    {
        if(flag == 0)   flag = FILTER_TIMES;
    }
    if((flag > 0) && (IO_KEY == 0))
    {
        flag --;
        key_value = KEY_SWITCH;
        if(key_value_reg == key_value)
        {
            if(flag == 0) return KEY_SWITCH;
        }
        else flag = FILTER_TIMES;
    }
    else if(IO_KEY)
    {
        flag = FILTER_TIMES;
    }

    key_value_reg = key_value;

    return KEY_NONE;
}

/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : ��ѭ��
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
        if(KEY_SWITCH == Key_Scan(0))
        {
            switch_sta = !switch_sta;
            tmos_set_event(Peripheral_TaskID, IOCHUB_SWITCH_CHANGE_EVT);
        }
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : ������
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
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
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    GPIOA_ModeCfg(LED_LIGHT_PIN | LED_LINK_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_SetBits(LED_LIGHT_PIN | LED_LINK_PIN);

    GPIOB_ModeCfg(KEY_SWITCH_PIN, GPIO_ModeIN_PU);

    CH59x_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
