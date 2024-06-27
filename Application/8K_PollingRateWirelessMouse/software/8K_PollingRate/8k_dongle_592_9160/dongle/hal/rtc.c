/********************************** (C) COPYRIGHT *******************************
* File Name          : RTC.c
* Author             : WCH
* Version            : V1.0
* Date               : 2018/11/06
* Description        : RTC配置及其初始化
*******************************************************************************/




/******************************************************************************/
/* 头文件包含 */
#include "HAL.h"


/***************************************************
 * Global variables
 */
volatile uint32_t RTCTigFlag;

#define US_TO_TICK(us)                  (uint32_t)((us)/(1000000/(CAB_LSIFQ)))
#define SLEEP_WAIT_HSE_TIME             US_TO_TICK(2400)
#define RTC_MAX_COUNT                   0xA8C00000

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
    RTCTigFlag = 0;
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
  Calibration_LSI( Level_128 );
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

/******************************** endfile @ time ******************************/
