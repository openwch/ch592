/********************************** (C) COPYRIGHT *******************************
* File Name          : MCU.c
* Author             : WCH
* Version            : V1.0
* Date               : 2018/11/12
* Description        : 硬件任务处理函数及BLE和硬件初始化
*******************************************************************************/




/******************************************************************************/
/* 头文件包含 */
#include "HAL.h"


/*********************************************************************
 * GLOBAL TYPEDEFS
 */
tmosTaskID halTaskID;

/*******************************************************************************
 * @fn          HAL_ProcessEvent
 *
 * @brief       硬件层事务处理
 *
 * input parameters
 *
 * @param       task_id.
 * @param       events.
 *
 * output parameters
 *
 * @param       events.
 *
 * @return      None.
 */
tmosEvents HAL_ProcessEvent( tmosTaskID task_id, tmosEvents events )
{
  if( events & HAL_TEST_EVENT ){
    {
    }
    return events ^ HAL_TEST_EVENT;
  }
  return 0;
}

/*******************************************************************************
 * @fn          Hal_Init
 *
 * @brief       硬件初始化
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
 void HAL_Init( void )
{
  halTaskID = TMOS_ProcessEventRegister( HAL_ProcessEvent );
  PRINT("hal init\n");
  HAL_TimeInit( );
//  tmos_start_reload_task( halTaskID , HAL_TEST_EVENT ,MS1_TO_SYSTEM_TIME(5000) );
}

/******************************** endfile @ mcu ******************************/
