/********************************** (C) COPYRIGHT *******************************
 * File Name          : rf_test.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        : rf收发测试例程，单向发送
 *                      PB15低电平为发送模式，默认为接收模式
 *
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/


/******************************************************************************/
/* 头文件包含 */
#include "ch9160.h"
#include <rf.h>
#include "CH59x_common.h"
#include "nvs_flash.h"
#include "hal.h"
#include "app.h"
#include "ota.h"


/*********************************************************************
 * GLOBAL TYPEDEFS
 */
#define  TMOS_BUF_SZE      1024

__attribute__((__aligned__(4))) uint8_t  TMOSBuf[TMOS_BUF_SZE];

__HIGH_CODE
static void SYS_EnableIrq(void)
{
    PFIC_EnableIRQ( BLEL_IRQn );
    PFIC_EnableIRQ(RTC_IRQn);
}
__HIGH_CODE
static void SYS_DisableIrq(void)
{
    PFIC_DisableIRQ( BLEL_IRQn );
    PFIC_DisableIRQ(RTC_IRQn);
}

__attribute__((used))
__HIGH_CODE
void process_main( void )
{
  while(1)
  {
      TMOS_SystemProcess();
      trans_process();
      app_data_process();
      app_retran_data_to_uart();
  }
}

/*******************************************************************************
* Function Name  : main
* Description    : 主函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main( void ) 
{
  SetSysClock( CLK_SOURCE_PLL_60MHz );
  SysTick_Config(0xFFFFFFFF);
  PFIC_DisableIRQ(SysTick_IRQn);
  mDelaymS(2);
  R32_PA_DIR &= 0;
  R32_PB_DIR &= 0;
  R32_PA_PU |= 0xFFFFFFFF;
  R32_PB_PU |= 0xFFFFF3FF;
#ifdef DEBUG_INFO
  GPIOA_SetBits( bTXD0_ );
  GPIOA_ModeCfg( bTXD0_, GPIO_ModeOut_PP_5mA );
  UART0_DefInit( );
  UART0_BaudRateCfg( 921600 );
  GPIOPinRemap(ENABLE,RB_PIN_UART0);
  GPIOA_SetBits(GPIO_Pin_9);

  GPIOA_SetBits( bAIN2 );
  GPIOA_ModeCfg( bAIN2, GPIO_ModeOut_PP_5mA );
#endif  
  PRINT("--- start\n");
  SoftwareUART_Printf("--- start\n");
  ch9160_Init();

  {
      tmosConfig_t conf={0};
      conf.MEMAddr = (uint32_t)TMOSBuf;
      conf.MEMLen = TMOS_BUF_SZE;
      conf.TaskMaxCount = 10;
      conf.enableTmosIrq = SYS_EnableIrq;
      conf.disableTmosIrq = SYS_DisableIrq;
      PRINT("tmos init.\n");
      TMOS_Init( &conf );
  }
  RFIP_PAControlInit(0);
  nvs_flash_init();
  HAL_Init( );
  access_Init();
  RF_Init( );
  app_Init();
  OTA_Init();
  process_main( );
}

/******************************** endfile @ main ******************************/
