/********************************** (C) COPYRIGHT *******************************
 * File Name          : rf.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        :
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/


/******************************************************************************/
/* 头文件包含 */
#include <RF.h>
#include "CH59x_common.h"
#include "wchrf.h"

/***************************************************
 * Global variables
 */

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
RF_DMADESCTypeDef DMARxDscrTab[RF_RXBUFNB];
RF_DMADESCTypeDef DMATxDscrTab[RF_TXBUFNB];

 __attribute__((__aligned__(4))) uint8_t  RFRxBuf[RF_RXBUFNB][ALIGNED4(RF_RX_BUF_SZE+PKT_HEAD_LEN+4)];
 __attribute__((__aligned__(4))) uint8_t  RFTxBuf[RF_TXBUFNB][ALIGNED4(RF_TX_BUF_SZE+PKT_HEAD_LEN+8)];

RF_DMADESCTypeDef *pDMARxGet;
RF_DMADESCTypeDef *pDMATxGet;

volatile uint8_t  gSleepFlag;
/*******************************************************************************
 * @fn
 *
 * @brief
 *
 * @param   None.
 *
 * @return  None.
 */
__INTERRUPT
__HIGH_CODE
void LLE_IRQHandler( void )
{
    LLE_LibIRQHandler( );
}

__INTERRUPT
__HIGH_CODE
void BB_IRQHandler( void )
{
    BB_LibIRQHandler( );
}

/*******************************************************************************
 * @fn      RTC_SetTignTime
 *
 * @brief   配置RTC触发时间
 *
 * @param   time    - 触发时间.
 *
 * @return  None.
 */
void RTC_SetTignTime(uint32_t time)
{
    sys_safe_access_enable();
    R32_RTC_TRIG = time;
    sys_safe_access_disable();
}

/*******************************************************************************
 * @fn          RTC_IRQHandler
 *
 * @brief       RTC中断处理
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
__INTERRUPT
__HIGH_CODE
void RTC_IRQHandler( void )
{
    volatile uint32_t i;
    uint32_t trig_time;
    R8_RTC_FLAG_CTRL =(RB_RTC_TMR_CLR|RB_RTC_TRIG_CLR);

    if(gSleepFlag)
    {
        i = RTC_GetCycle32k();
        while(i == RTC_GetCycle32k());
        gSleepFlag = 0;
        DelayUs(1000);
        RFIP_WakeUpRegInit();
    }
}



/*******************************************************************************
 * @fn      Lib_Calibration_LSI
 *
 * @brief   内部32k校准
 *
 * @param   None.
 *
 * @return  None.
 */
void Lib_Calibration_LSI(void)
{
    Calibration_LSI(Level_64);
}

/**
 * @brief
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
__HIGH_CODE
static uint32_t SYS_GetClockValue(void)
{
  uint32_t volatile rtc_count;
  do{
      rtc_count = R32_RTC_CNT_32K;
  }while( rtc_count != R32_RTC_CNT_32K);
  return rtc_count;
}

__HIGH_CODE
static uint32_t SYS_GetClock1Value(void)
{
    return SysTick->CNT;
}
__HIGH_CODE
static void SYS_SetPendingIRQ(void)
{
    PFIC_SetPendingIRQ( TMR3_IRQn );
}
__HIGH_CODE
static void SYS_SetTignOffest( int32_t val )
{
    R32_TMR3_CNT_END += (val-R32_TMR3_COUNT);
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   TMR0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR3_IRQHandler(void) // TMR3
{
    uint32_t trig_time;

    TMR3_ClearITFlag(TMR0_3_IT_CYC_END); // 清除中断标志
    if( !TMOS_TimerIRQHandler( &trig_time )  )
    {
        if( trig_time ){
            R32_TMR3_CNT_END = trig_time;
            R8_TMR3_CTRL_MOD = RB_TMR_ALL_CLEAR;
            R8_TMR3_CTRL_MOD = RB_TMR_COUNT_EN;
        }
        else{
            PRINT("!!!!!!!!!!!!!!!!!! warn \n");
        }
    }
}

/*******************************************************************************
 * @fn          HAL_Time0Init
 *
 * @brief       系统定时器初始化
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void HAL_TimeInit( void )
{
    tmosTimeConfig_t conf;

#if( CLK_OSC32K )
  R8_SAFE_ACCESS_SIG = 0x57;
  R8_SAFE_ACCESS_SIG = 0xa8;
  R8_CK32K_CONFIG |= RB_CLK_INT32K_PON;
  R8_SAFE_ACCESS_SIG = 0;
  Calibration_LSI( Level_64 );
#else
  R8_SAFE_ACCESS_SIG = 0x57;
  R8_SAFE_ACCESS_SIG = 0xa8;
  R8_CK32K_CONFIG   |= RB_CLK_OSC32K_XT | RB_CLK_INT32K_PON | RB_CLK_XT32K_PON;
  R8_SAFE_ACCESS_SIG = 0;
#endif
  RTC_InitTime( 2021,1,28,0,0,0 );
  SysTick_Config(0xFFFFFFFF);
  PFIC_DisableIRQ(SysTick_IRQn);

  // tmos时间相关配置
  conf.ClockAccuracy = 500;
  conf.ClockFrequency = CAB_LSIFQ;
  conf.ClockMaxCount = RTC_MAX_COUNT;
  conf.getClockValue = SYS_GetClockValue;

  // rf通信时间相关配置
  conf.Clock1Frequency = GetSysClock( )/1000;  //kHz
  conf.getClock1Value = SYS_GetClock1Value;
  conf.SetPendingIRQ = SYS_SetPendingIRQ;
  conf.SetTign = SYS_SetTignOffest;
  TMOS_TimerInit( &conf );

  TMR3_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // 开启中断
  PFIC_EnableIRQ(TMR3_IRQn);
}

/*******************************************************************************
 * @fn      rfRoleBoundProcess
 *
 * @brief   RF收发DMA初始化
 *
 * @param   None.
 *
 * @return  None.
 */
void rfDMADescInit( void )
{
    int num;

    for( num=0;num<RF_RXBUFNB;num++ )
    {
        DMARxDscrTab[num].Status = STA_DMA_ENABLE;
        DMARxDscrTab[num].BufferSize = RF_RX_BUF_SZE+PKT_HEAD_LEN;
        DMARxDscrTab[num].BufferAddr = (uint32_t)RFRxBuf[num];
        DMARxDscrTab[num].NextDescAddr = (uint32_t)&DMARxDscrTab[num+1];
    }
    DMARxDscrTab[RF_RXBUFNB-1].NextDescAddr = (uint32_t)&DMARxDscrTab[0];
    pDMARxGet = DMARxDscrTab;

    for( num=0;num<RF_TXBUFNB;num++ )
    {
        DMATxDscrTab[num].Status = 0;
        DMATxDscrTab[num].BufferSize = RF_TX_BUF_SZE+PKT_HEAD_LEN;
        DMATxDscrTab[num].BufferAddr = (uint32_t)RFTxBuf[num];
        DMATxDscrTab[num].NextDescAddr = (uint32_t)&DMATxDscrTab[num+1];
    }
    DMATxDscrTab[RF_TXBUFNB-1].NextDescAddr = (uint32_t)&DMATxDscrTab[0];
    pDMATxGet = DMATxDscrTab;
}

/*******************************************************************************
 * @fn      RF_LibInit
 *
 * @brief   rf 库初始化
 *
 * @param   None.
 *
 * @return  None.
 */
void RF_LibInit( pfnRfRoleProcess cb )
{
    rfRoleConfig_t conf={0};
    rfRoleParam_t gParm={0};

    rfDMADescInit( );
    conf.TxPower = LL_TX_POWEER_4_DBM;
    conf.pTx = DMATxDscrTab;
    conf.pRx = DMARxDscrTab;
    conf.rfProcessCB = cb;
#if(defined HAL_SLEEP) && (HAL_SLEEP == TRUE)
    conf.processMask = RF_STATE_RX|RF_STATE_RBU|RF_STATE_TX_FINISH|RF_STATE_TX_IDLE;
#else
    conf.processMask = RF_STATE_RX|RF_STATE_RBU|RF_STATE_TX_IDLE;
#endif
    RFRole_LibInit( &conf );

    gParm.accessAddress = 0x71763988;
    gParm.crcInit  = 0x555555;
    gParm.frequency = 3;
    gParm.properties = PHY_MODE;
    gParm.rxMaxLen = RF_RX_BUF_SZE+PKT_ACK_LEN;
    PRINT( "properties=%x\n",gParm.properties );
    RFRole_SetParam( &gParm );
    PFIC_EnableIRQ( BLEB_IRQn );
    PFIC_EnableIRQ( BLEL_IRQn );
}


/******************************** endfile @rf ******************************/
