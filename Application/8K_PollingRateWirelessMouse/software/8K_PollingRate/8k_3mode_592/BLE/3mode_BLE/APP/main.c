/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
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
#include "CONFIG.h"
#include "HAL.h"
#include "hiddev.h"
#include "access.h"
#include "hidkbd.h"
#include "mouse.h"


/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

#define IAP_SAFE_FLAG_2_4G       0x30de5820
#define IAP_SAFE_FLAG_BLE        0x30de5821
#define IAP_SAFE_FLAG_MASK       0x30de5820

/* 用于APP判断文件有效性 */
__attribute__((aligned(4))) uint32_t save_Flag __attribute__((section(".ImageFlag"))) = IAP_SAFE_FLAG_BLE;

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
        peripheral_process();
    }
}

/*********************************************************************
 * @fn      Mode_Init
 *
 * @brief   Mode_Init
 *
 * @return  none
 */
void Mode_Init()
{
    nvs_flash_init();

    start_device_over = FALSE;
    CH59X_BLEInit(BLE_SNV_ADDR+0x100);
    DelayUs(1);
    HAL_Init();
    GAPRole_PeripheralInit( );
    HidDev_Init();
    HidEmu_Init();
    while(!start_device_over)
    {
        TMOS_SystemProcess();
    }

    start_device_over = FALSE;
    CH59X_BLEInit(BLE_SNV_ADDR+0x200);
    DelayUs(2);
    HAL_Init();
    GAPRole_PeripheralInit( );
    HidDev_Init();
    HidEmu_Init();
    while(!start_device_over)
    {
        TMOS_SystemProcess();
    }

    start_device_over = FALSE;
    CH59X_BLEInit(BLE_SNV_ADDR+0x300);
    DelayUs(3);
    HAL_Init();
    GAPRole_PeripheralInit( );
    HidDev_Init();
    HidEmu_Init();
    while(!start_device_over)
    {
        TMOS_SystemProcess();
    }

    start_device_over = FALSE;
    CH59X_BLEInit(BLE_SNV_ADDR+0x400);
    DelayUs(4);
    HAL_Init();
    GAPRole_PeripheralInit( );
    HidDev_Init();
    HidEmu_Init();
    while(!start_device_over)
    {
        TMOS_SystemProcess();
    }

    CH59X_BLEInit(BLE_SNV_ADDR);
    DelayUs(5);
    HAL_Init();
    GAPRole_PeripheralInit( );
    RF_RoleInit( );
    HidDev_Init();
    HidEmu_Init();
    access_init();
    peripheral_init();

    llRecvDataDisable |=2;
    PRINT("BLE MODE\n");
}

/*********************************************************************
 * @fn      trans_RF_receive
 *
 * @brief   trans_RF_receive
 *
 * @return  none
 */
void trans_RF_receive( uint8_t *pData, uint8_t len )
{
    PRINT("RF %x\n",pData[0] );
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
    PowerMonitor(ENABLE, LPLevel_2V5);
//    PFIC_EnableIRQ(WDOG_BAT_IRQn);
    PWR_DCDCCfg(ENABLE);
#if (defined (HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All ,GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All & (~(VDR_R|VDR_G|VDR_B)) ,GPIO_ModeIN_PU);
#endif
#if (defined (DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    if(!GPIOB_ReadPortPin(GPIO_Pin_4))
    {
        PWR_DCDCCfg(ENABLE);
    }
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);

#ifdef DEBUG
    GPIOB_SetBits( bTXD2 );
    GPIOB_ModeCfg( bTXD2, GPIO_ModeOut_PP_5mA );
    UART2_DefInit( );
    UART2_BaudRateCfg( 921600 );
#endif

#if CONFIG_RF_DEBUG_GPIO
    GPIOA_ResetBits(GPIO_Pin_0 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6);
    GPIOA_ModeCfg(GPIO_Pin_0 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6, GPIO_ModeOut_PP_5mA);
#endif

    PRINT("start. %s\n",__DATE__);
    PRINT("%s\n", VER_LIB);
    PRINT("reset: %x\n", SYS_GetLastResetSta());

    GPIOA_ResetBits(KEY_COM);
    GPIOA_ModeCfg(KEY_COM, GPIO_ModeOut_PP_20mA);
    GPIOB_ModeCfg(KEY_BT|KEY_2_4G, GPIO_ModeIN_PU);

    if(!KEY_BT_ST)
    {
        work_mode = MODE_BT;
    }
    else if(!KEY_2_4G_ST)
    {
        work_mode = MODE_2_4G;
        SYS_ResetExecute();
    }
    else
    {
        work_mode = MODE_USB;
        SYS_ResetExecute();
    }
    PRINT("work_mode %x\n",work_mode);

    Mode_Init();
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
