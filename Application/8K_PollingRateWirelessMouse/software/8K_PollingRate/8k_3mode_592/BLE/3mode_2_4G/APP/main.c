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
#include <rf.h>
#include "rf_device.h"
#include "CH59x_common.h"
#include "ota.h"
#include "usb.h"
#include "trans.h"
#include "mouse.h"
#include "peripheral.h"


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
      peripheral_process();
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
#ifdef DEBUG
  GPIOB_SetBits( bTXD2 );
  GPIOB_ModeCfg( bTXD2, GPIO_ModeOut_PP_5mA );
  UART2_DefInit( );
  UART2_BaudRateCfg( 921600 );
#else
  R8_UART2_IER = 0;
#endif
  PRINT("--- start\n");

  {
      tmosConfig_t conf ={0};
      conf.MEMAddr = (uint32_t)TMOSBuf;
      conf.MEMLen = TMOS_BUF_SZE;
      conf.TaskMaxCount = 10;
      conf.enableTmosIrq = SYS_EnableIrq;
      conf.disableTmosIrq = SYS_DisableIrq;
      PRINT("tmos init.\n");
      TMOS_Init( &conf );
  }
  nvs_flash_init();
  HAL_TimeInit( );
  GPIOA_ResetBits(KEY_COM);
  GPIOA_ModeCfg(KEY_COM, GPIO_ModeOut_PP_20mA);
  GPIOB_ModeCfg(KEY_BT|KEY_2_4G, GPIO_ModeIN_PU);

#if 1
  if(!KEY_BT_ST)
  {
      work_mode = MODE_BT;
      SYS_ResetExecute();
  }
  else if(!KEY_2_4G_ST)
  {
      work_mode = MODE_2_4G;
      GPIOB_ModeCfg(GPIO_Pin_10|GPIO_Pin_11 ,GPIO_ModeIN_PU);
  }
  else
  {
      work_mode = MODE_USB;
  }
#else
  work_mode = MODE_2_4G;
  GPIOB_ModeCfg(GPIO_Pin_10|GPIO_Pin_11 ,GPIO_ModeIN_PU);

#endif

  PRINT("work_mode %x  %x %x\n",work_mode,KEY_BT_ST,KEY_2_4G_ST);
//  PRINT("TEST: set work_mode USB\n");
//  work_mode = MODE_USB;
  trans_Init();
  peripheral_init();
  if(work_mode == MODE_USB)
  {
      USB_Init();
  }
  else if(work_mode == MODE_2_4G)
  {
      RF_Init( );
  }
  OTA_Init();
  process_main( );
}

/******************************** endfile @ main ******************************/
