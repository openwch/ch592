/********************************** (C) COPYRIGHT *******************************
 * File Name          : rf_device.c
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
#include <SLEEP.h>
#include "rf_device.h"
#include "CH59x_common.h"
#include "ota.h"
#include "mouse.h"
#include "peripheral.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define  RF_TEST_PERIDOC   500  // 2K

#define RF_REPORT_DISCONNECT_DELAY      (160*4)//延迟上报断开，防止睡眠唤醒的短暂断连影响状态上报

RF_con_status_t RF_state;

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

uint32_t gErrContinue = 0;
uint32_t gErrCount = 0;
uint32_t gTxCount = 0;
uint32_t gRxCount = 0;
uint8_t gTestCount = 0;
uint8_t gTestData = 0;

uint32_t RFMode;
#define  RF_MODE_RX  0
#define  RF_MODE_TX  1

extern RF_DMADESCTypeDef DMATxDscrTab[RF_TXBUFNB];
int8_t rssi = 0;
tmosTaskID rfTaskID;
uint8_t gDeviceId;

uint8_t self_mac[6] = {0};

uint8_t gRetry = 0;

/*********************************************************************
 * @fn      RF_check_con_status
 *
 * @brief   检查当前连接状态
 *
 * @return  none
 */
__HIGH_CODE
uint8_t RF_check_con_status(RF_con_status_t status)
{
    if(RF_state==status)
        return (TRUE) ;
    else
        return (FALSE) ;
}

/*********************************************************************
 * @fn      RF_set_con_status
 *
 * @brief   设置当前连接状态
 *
 * @return  none
 */
__HIGH_CODE
void RF_set_con_status(RF_con_status_t status)
{
    RF_state=status;
}

/*******************************************************************************
 * @fn      rf_get_txbuf_num
 *
 * @brief   RF 获取当前发送缓冲中的包个数
 *
 * @param   None.
 *
 * @return  None.
 */
uint8_t rf_get_txbuf_num( void )
{
    int num;
    uint8_t count=0;
    for( num = 0; num < RF_TXBUFNB; num++ )
    {
        if(DMATxDscrTab[num].Status & STA_DMA_ENABLE)
        {
            count++;
        }
    }
    return count;
}

/*******************************************************************************
 * @fn      rf_get_data
 *
 * @brief   获取一包RF接收DMA中的数据
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
uint8_t *rf_get_data( uint8_t *pLen)
{
    if(!(pDMARxGet->Status & STA_DMA_ENABLE))
    {
        *pLen = (pDMARxGet->Status & STA_LEN_MASK) - PKT_HEAD_LEN;
        return (uint8_t *)(pDMARxGet->BufferAddr + PKT_HEAD_LEN);
    }
    else {
        return 0;
    }
}

/*******************************************************************************
 * @fn      rf_delete_data
 *
 * @brief   删除一包RF接收DMA中的数据
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void rf_delete_data()
{
    pDMARxGet->Status = STA_DMA_ENABLE;
    pDMARxGet = (RF_DMADESCTypeDef *) pDMARxGet->NextDescAddr;
}

/*******************************************************************************
 * @fn      rf_send_data
 *
 * @brief   向RF发送DMA中添加一包数据
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
uint8_t rf_send_data( uint8_t *pData, uint8_t len)
{
    uint32_t irqv;
    SYS_DisableAllIrq( &irqv );
    if( !(pDMATxGet->Status & STA_DMA_ENABLE) )
    {
        rfPackage_t *p = (rfPackage_t *) pDMATxGet->BufferAddr;
        uint8_t *pPdu;

        p->type = gDeviceId;
        p->length = PKT_ACK_LEN + len;
        pPdu = (uint8_t *) (pDMATxGet->BufferAddr + PKT_HEAD_LEN);
        tmos_memcpy(pPdu, pData, len);
        pDMATxGet->BufferSize = len + PKT_HEAD_LEN;
        pDMATxGet->Status = STA_DMA_ENABLE;
        pDMATxGet = (RF_DMADESCTypeDef *) pDMATxGet->NextDescAddr;
        SYS_RecoverIrq( irqv );
        return 0;
    }
    SYS_RecoverIrq( irqv );
    return 0xFF;
}


/*******************************************************************************
 * @fn      RF_ProcessCallBack
 *
 * @brief   RF 状态回调，注意此函数为中断中调用
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void RF_ProcessCallBack( rfRole_States_t sta,uint8_t id  )
{
    // 收到数据回调
    if( sta & RF_STATE_RX )
    {
        tmos_set_event( rfTaskID, RF_RECV_PROCESS_EVENT );
    }
    // 接收DMA满
    if( sta & RF_STATE_RBU )
    {
        PRINT( "!rbu\n" );
    }
    if( sta & RF_STATE_TX_FINISH )
    {
        gErrContinue = 0;
    }
    if( sta & RF_STATE_TX_IDLE )
    {

    }
//    if( sta & RF_STATE_RX_RETRY )
//    {
//        gRetry = TRUE;
//        tmos_set_event( rfTaskID, RF_SLEEP_EVENT );
//    }
}

/*******************************************************************************
 * @fn      rfRoleBoundProcess
 *
 * @brief   连接绑定状态回调
 *
 * @param   None.
 *
 * @return  None.
 */
void rfRoleBoundProcess( staBound_t *pSta )
{
    PRINT( "bound %x\n",pSta->status );
//    PRINT( "status=%x\n", pSta->status );
//    PRINT( "role=%x\n", pSta->role );
//    PRINT( "id=%x\n", pSta->devId );
//    PRINT( "type=%x\n", pSta->devType );
//    PRINT( "hop=%x\n", pSta->hop );
//    PRINT( "mac=%x %x %x %x %x %x\n", pSta->PeerInfo[0], pSta->PeerInfo[1], pSta->PeerInfo[2], pSta->PeerInfo[3], pSta->PeerInfo[4], pSta->PeerInfo[5] );
    if( !pSta->status )
    {
//        tmos_start_reload_task(rfTaskID, RF_TEST_TX_EVENT, 1600);
        RF_set_con_status(RF_CON_CONNECTED);
        gDeviceId = pSta->devId;
        // 判断当前RF角色
        if( !(pSta->role&1) )
        {
            // 当前为接收端（dongle）
        }
        else
        {
            // 当前为发送端（鼠标）
            if((nvs_flash_info.rf_device_id!=pSta->devId) ||
                (!tmos_memcmp(nvs_flash_info.peer_mac, pSta->PeerInfo, 6)))
            {
                nvs_flash_info.rf_device_id = pSta->devId;
                tmos_memcpy(nvs_flash_info.peer_mac, pSta->PeerInfo, 6);
                nvs_flash_store();
//                tmos_start_task( rfTaskID, RF_TEST_RX_EVENT, 1600 );
            }
        }
        if(!tmos_get_task_timer(rfTaskID, RF_REPORT_DISCONNECT_EVENT))
        {
            PRINT("CONNECTED\n");
            peripheral_connected_cb();
        }
        tmos_stop_task(rfTaskID, RF_REPORT_DISCONNECT_EVENT);
    }
    else
    {
        if( !(pSta->role&1) )
        {
            // 当前为接收端（dongle）
        }
        else
        {
            // 当前为发送端（鼠标）
        }
        // RF自动回连超时失败
        if( pSta->status == FAILURE )
        {
            if(RF_check_con_status(RF_CON_CONNECTED))
            {
                tmos_start_task(rfTaskID, RF_REPORT_DISCONNECT_EVENT, RF_REPORT_DISCONNECT_DELAY);
            }
            else {
            }
            RF_set_con_status(RF_CON_IDEL);
            tmos_set_event( rfTaskID, RF_START_BOUND_EVENT );
        }
        // RF当前连接断开，自动启用回连
        else if( pSta->status == bleTimeout )
        {
#if(CLK_OSC32K)
            Lib_Calibration_LSI(); // 校准内部RC
#endif
            PRINT( "t o.\n" );
            tmos_start_task(rfTaskID, RF_REPORT_DISCONNECT_EVENT, RF_REPORT_DISCONNECT_DELAY);
        }
    }
}

/*******************************************************************************
 * @fn      RF_ProcessEvent
 *
 * @brief   RF层系统任务处理
 *
 * @param   None.
 *
 * @return  None.
 */
tmosEvents RF_ProcessEvent( tmosTaskID task_id, tmosEvents events )
{
    if( events & SYS_EVENT_MSG )
    {
        uint8_t * msgPtr;

        msgPtr = tmos_msg_receive( task_id );
        if( msgPtr )
        {
            /* De-allocate */
            tmos_msg_deallocate( msgPtr );
        }
        return events ^ SYS_EVENT_MSG;
    }

    if( events & RF_START_BOUND_EVENT )
    {
        rfBoundDevice_t bound;

        gRetry = 0;
        tmos_memset( &bound, 0, sizeof(rfBoundDevice_t) );
//        PRINT("start tx mode\n");
#if (KEYBOARD)
        bound.devType = DEVICE_KEYBOARD_TYPE;
        bound.deviceId = DEVICE_KEYBOARD; // RF_ROLE_ID_INVALD表示由dongle端决定ID号，也可指定ID号
#elif (MOUSE)
        bound.devType = DEVICE_MOUSE_TYPE;
        bound.deviceId = RF_ROLE_ID_INVALD; // RF_ROLE_ID_INVALD表示由dongle端决定ID号，也可指定ID号
#else
        PRINT("devType ERR!\n");
        while(1);
#endif
//        PRINT( "s id %x\n", bound.deviceId );
        bound.timeout = 150;
        bound.speed = 12;
        tmos_memcpy(bound.OwnInfo, self_mac, 6);
        tmos_memcpy(bound.PeerInfo, nvs_flash_info.peer_mac, 6);
        bound.rfBoundCB = rfRoleBoundProcess;
        RFBound_StartDevice( &bound );
        RF_set_con_status(RF_CON_PAIRING);
        return events ^ RF_START_BOUND_EVENT;
    }

    if( events & RF_RECV_PROCESS_EVENT )
    {
        uint8_t len;
        uint8_t *pData;
        pData = rf_get_data(&len);
        if(pData)
        {
            if(pData[0] == RF_DATA_IAP)
            {
                peripheral_sleep_update();
                OTA_USB_IAPWriteData(&pData[2], len-2);
            }
            else if(pData[0] == RF_DATA_LED)
            {
                PRINT("led %x\n",pData[2]);
                peripheral_pilot_led_receive(pData[2]);
            }
            else if(pData[0] == RF_DATA_MANUFACTURER)
            {
                PRINT("MANUFACTURER %x\n",pData[2]);
                peripheral_sleep_update();
            }
            else if(pData[0] == RF_DATA_SLEEP)
            {
                PRINT("SLEEP %x\n",pData[2]);
                if( pData[2] == ENTER_SLEEP)
                {
                    deep_sleep_flag = 1;
                    tmos_start_task( rfTaskID, RF_SLEEP_EVENT, 160 );
                }
            }
            else {
              PRINT("??? %x\n",pData[0]);
            }
            rf_delete_data();
            tmos_set_event( rfTaskID, RF_RECV_PROCESS_EVENT );
        }
        return events ^ RF_RECV_PROCESS_EVENT;
    }

    if( events & RF_SLEEP_EVENT )
    {
        if(deep_sleep_flag)
        {
            PRINT("deep s\n");
            RFRole_Shut();
            peripheral_enter_sleep();
            PWR_PeriphWakeUpCfg( DISABLE, RB_SLP_RTC_WAKE, Long_Delay );
//            LowPower_Sleep( RB_PWR_RAM24K | RB_PWR_RAM2K | RB_PWR_EXTEND ); //只保留24+2K SRAM 供电
            HSECFG_Current(HSE_RCur_100); // 降为额定电流(低功耗函数中提升了HSE偏置电流)
//            LowPower_Shutdown(RB_PWR_RAM2K);
//            SYS_ResetExecute();
            RFIP_WakeUpRegInit();
            PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_RTC_WAKE, Long_Delay );
            tmos_set_event( rfTaskID, RF_START_BOUND_EVENT );
        }
        else if(idel_sleep_flag)
        {
            if(gRetry)
            {
                gRetry = 0;
                RFRole_Shut();
                peripheral_enter_sleep();
                RF_LowPower(MS_TO_RTC(135));
            }
            else if(!RF_check_con_status(RF_CON_CONNECTED))
            {
                RF_stop();
                peripheral_enter_sleep();
                PWR_PeriphWakeUpCfg( DISABLE, RB_SLP_RTC_WAKE, Long_Delay );
                LowPower_Shutdown(RB_PWR_RAM2K);
                SYS_ResetExecute();
                RFIP_WakeUpRegInit();
                PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_RTC_WAKE, Long_Delay );
                PRINT("w\n");
            }
        }

        return events ^ RF_SLEEP_EVENT;
    }

    if( events & RF_REPORT_DISCONNECT_EVENT )
    {
        PRINT( "REPORT disconnect.\n" );
        peripheral_sleep_update();
        peripheral_disconnected_cb();
        tmos_stop_task(rfTaskID, RF_TEST_TX_EVENT);
        return events ^ RF_REPORT_DISCONNECT_EVENT;
    }

    if( events & RF_TEST_TX_EVENT )
    {
        PRINT("tx status-%d %d\n", gErrCount, gTxCount );
        gErrCount = 0;
        gTxCount = 0;
        return events ^ RF_TEST_TX_EVENT;
    }

    if( events & RF_TEST_RX_EVENT )
    {
        nvs_flash_store();
        return events ^ RF_TEST_RX_EVENT;
    }
    return 0;
}

/*******************************************************************************
 * @fn      RF_ReStart
 *
 * @brief   RF重新初始化
 *
 * @param   None.
 *
 * @return  None.
 */
void RF_ReStart( void )
{
    RFRole_Shut();
    RF_LibInit(RF_ProcessCallBack);
    peripheral_disconnected_cb();
    tmos_set_event( rfTaskID, RF_START_BOUND_EVENT );
    if(tmos_isbufset(nvs_flash_info.peer_mac, 0, 6))
    {
        peripheral_pairing_cb();
    }
    else {
        peripheral_connecting_cb();
    }
}

void access_enter_idel_sleep( void )
{
    tmos_set_event( rfTaskID, RF_SLEEP_EVENT );
}

void access_enter_deep_sleep( void )
{
    tmos_set_event( rfTaskID, RF_SLEEP_EVENT );
}

__HIGH_CODE
void access_weakup( void )
{
    if((work_mode == MODE_2_4G)&&(!RF_check_con_status(RF_CON_CONNECTED)) && (deep_sleep_flag))
        tmos_set_event( rfTaskID, RF_START_BOUND_EVENT );
}

/*******************************************************************************
 * @fn      RF_stop
 *
 * @brief   RF_stop
 *
 * @param   None.
 *
 * @return  None.
 */
void RF_stop( void )
{
    RFRole_Shut();
    RF_LibInit(RF_ProcessCallBack);
    RF_set_con_status(RF_CON_IDEL);
}

/*******************************************************************************
 * @fn      RF_Init
 *
 * @brief   RF应用层初始化
 *
 * @param   None.
 *
 * @return  None.
 */
void RF_Init( void )
{
    rfTaskID = TMOS_ProcessEventRegister( RF_ProcessEvent );

    PRINT( "rf mode----------- Tx -----------\n" );
    PRINT( "%s\n", VER_LIB );
    GetMACAddress(self_mac);

    gErrContinue = 0;
    gErrCount = 0;
    gTxCount = 0;
    gRxCount = 0;
    gTestCount = 0;
    gTestData = 0;
    RF_LibInit(RF_ProcessCallBack);
    tmos_set_event( rfTaskID, RF_START_BOUND_EVENT );
    RF_set_con_status(RF_CON_IDEL);
    peripheral_connecting_cb();
    RF_SleepInit();
}


/******************************** endfile @rf ******************************/
