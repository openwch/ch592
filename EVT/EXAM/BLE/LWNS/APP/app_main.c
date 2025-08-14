/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2025/04/27
 * Description        : lwns主程序
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/


/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "CH59x_common.h"
#include "HAL.h"
#include "stdlib.h"
#include "lwns_adapter_csma_mac.h"
#include "lwns_adapter_blemesh_mac.h"
#include "lwns_adapter_no_mac.h"
#include "lwns_broadcast_example.h"
#include "lwns_ruc_example.h"
#include "lwns_rucft_example.h"
#include "lwns_unicast_example.h"
#include "lwns_multicast_example.h"
#include "lwns_netflood_example.h"
#include "lwns_uninetflood_example.h"
#include "lwns_multinetflood_example.h"
#include "lwns_mesh_example.h"

/* 每个文件单独debug打印的开关，置0可以禁止本文件内部打印 */
#define DEBUG_PRINT_IN_THIS_FILE    1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...)       PRINT(__VA_ARGS__)
#else
#define PRINTF(...)       do{} while(0)
#endif

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if (defined (BLE_MAC)) && (BLE_MAC == TRUE)
uint8_t const MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

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
    while (1)
    {
        TMOS_SystemProcess();
    }
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
#if (defined (DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#if (defined (HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
#ifdef DEBUG
    GPIOA_SetBits( bTXD1 );
    GPIOA_ModeCfg( bTXD1, GPIO_ModeOut_PP_5mA );
    UART1_DefInit( );
#endif
    PRINTF("start.\n");
    {
        PRINTF("%s\n", VER_LIB);
    }
    CH59x_BLEInit();
    HAL_Init();
    RF_RoleInit();
    RF_Init();
    lwns_init();                          /* 初始lwns协议栈 */
    //lwns_broadcast_process_init();      /* 广播 */
    //lwns_multicast_process_init();      /* 组播 */
    //lwns_unicast_process_init();        /* 单播 */
    //lwns_ruc_process_init();            /* 可靠单播 */
    //lwns_rucft_process_init();          /* 可靠单播文件传输 */
    lwns_netflood_process_init();         /* 网络泛洪 */
    //lwns_uninetflood_process_init();    /* 单播网络泛洪 */
    //lwns_multinetflood_process_init();  /* 组播网络泛洪 */
    //lwns_mesh_process_init();           /* mesh组网 */
    Main_Circulation();
}
/******************************** endfile @ main ******************************/
