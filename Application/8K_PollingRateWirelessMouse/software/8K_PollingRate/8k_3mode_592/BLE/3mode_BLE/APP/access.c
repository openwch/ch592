/********************************** (C) COPYRIGHT *******************************
 * File Name          : access.c
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
#include "access.h"
#include "battservice.h"
#include "hiddev.h"
#include "hidkbd.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define SEND_ONE_REPORT_TIMEOUT    4

uint8_t last_led_data = 0xFF;

access_state_t access_state;

// Task ID
uint8_t access_taskId = INVALID_TASK_ID;

uint16_t access_ProcessEvent( uint8_t task_id, uint16_t events );
void access_receive_cb( uint8_t *pData, uint8_t len );
void access_tran_report( uint8_t cmd, uint8_t data );
void access_ctl_process( uint8_t ctl_type );
void access_switch_ble_mode( void );
void access_pairing_mode( void );

/*********************************************************************
 * @fn      access_init
 *
 * @brief   access_init
 *
 * @return  none
 */
void access_init()
{
    access_state.ble_idx = BLE_INDEX_IDEL;
    access_state.pairing_state = FALSE;
    access_state.sleep_en = FALSE;
    access_state.deep_sleep_flag = FALSE;
    access_state.idel_sleep_flag = FALSE;
    access_taskId = TMOS_ProcessEventRegister( access_ProcessEvent );

    HSECFG_Capacitance(nvs_flash_info.capacitance);

    hidEmu_receive_cb_register( access_receive_cb );

//    TMR1_TimerInit( 60000000 );
//    TMR1_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
//    PFIC_EnableIRQ( TMR1_IRQn );

}

/*********************************************************************
 * @fn      access_ProcessEvent
 *
 * @brief   access Task event processor.
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t access_ProcessEvent( uint8_t task_id, uint16_t events )
{

    if( events & ACCESS_SLEEP_EVT )
    {
        PRINT( "ACCESS_SLEEP_EVT \n" );
        // 深度睡眠，不开广播不连接
        access_state.sleep_en = TRUE;
        access_state.deep_sleep_flag = TRUE;
        tmos_stop_task(access_taskId, ACCESS_IDEL_SLEEP_EVT);
        tmos_stop_task( access_taskId, ACCESS_WAKE_UP_EVT );
        if((access_state.ble_idx > BLE_INDEX_IDEL) && (access_state.ble_idx < BLE_INDEX_MAX))
        {
            uint8_t ble_state;
            GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
            PRINT( "SLEEP_EVT state %x\n", ble_state );
            if( ble_state == GAPROLE_CONNECTED )
            {
                hidEmu_disconnect();
                tmos_start_task(access_taskId, ACCESS_SLEEP_EVT, DISCONNECT_WAIT_REPORT_END_TIMEOUT);
                return (events ^ ACCESS_SLEEP_EVT);
            }
            if( adv_enable_process_flag )
            {
                tmos_start_task(access_taskId, ACCESS_SLEEP_EVT, DISCONNECT_WAIT_REPORT_END_TIMEOUT);
                return (events ^ ACCESS_SLEEP_EVT);
            }
            hidEmu_adv_enable( DISABLE );
            access_state.ble_idx = BLE_INDEX_IDEL;
        }
        hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
        PRINT( "real sleep\n       " );
//        access_state.idel_sleep_flag = FALSE;
//        peripheral_enter_sleep();
        return (events ^ ACCESS_SLEEP_EVT);
    }
    if( events & ACCESS_IDEL_SLEEP_EVT )
    {
        PRINT("S@\n");
        // 进入睡眠 ，ble保持连接
        access_state.sleep_en = TRUE;
        access_state.idel_sleep_flag = TRUE;
        if((access_state.ble_idx > BLE_INDEX_IDEL) && (access_state.ble_idx < BLE_INDEX_MAX))
        {
            uint8_t ble_state;
            GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
            PRINT( "S@ state %x\n", ble_state );
            if( ble_state == GAPROLE_CONNECTED )
            {
                //正常连接
            }
            else if( ble_state == GAPROLE_ADVERTISING )
            {
                // 停止广播，并不再广播。
                access_state.ble_idx = BLE_INDEX_IDEL;
                hidEmu_adv_enable( DISABLE );
            }
            else
            {
                //没在广播没有连接，
                access_state.ble_idx = BLE_INDEX_IDEL;
            }
        }
        hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
//        peripheral_enter_sleep();
        return (events ^ ACCESS_IDEL_SLEEP_EVT);
    }

    if( events & ACCESS_WAKE_UP_EVT )
    {
        if(access_state.ble_idx == BLE_INDEX_IDEL)
        {
            access_state.ble_idx = con_work_mode;
            if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
            {
                hidEmu_adv_enable( ENABLE );
            }
        }
        else {
//            PRINT("W mode %x\n",access_state.ble_idx);
        }
        PRINT("W@\n");
        if(  access_state.deep_sleep_flag )
        {
//            access_tran_report( REPORT_CMD_STATE, STATE_WAKE_UP );
        }
        access_state.deep_sleep_flag = FALSE;
        access_state.idel_sleep_flag = FALSE;
        access_state.sleep_en = FALSE;
        hidDevBattCB(BATT_LEVEL_NOTI_ENABLED);
        return (events ^ ACCESS_WAKE_UP_EVT);
    }

    return 0;
}

/*********************************************************************
 * @fn      access_get_checksum
 *
 * @brief   access_get_checksum
 *
 * @return  none
 */
uint8_t access_get_checksum( uint8_t *pData, uint8_t len )
{
    int i;
    uint8_t checksum = 0;
    for( i = 0; i < len; i++ )
    {
        checksum += pData[i];
    }
    return checksum;
}

/*********************************************************************
 * @fn      access_receive_cb
 *
 * @brief   access_receive_cb
 *
 * @return  none
 */
void access_receive_cb( uint8_t *pData, uint8_t len )
{
    if(pData[0] != RF_DATA_LED )
    {
//        access_update_idel_sleep_timeout(IDEL_SLEEP_EVT_TIMEOUT);
        if(access_state.idel_sleep_flag)
        {
            // 停止睡眠
            access_state.sleep_en = FALSE;
            tmos_set_event( access_taskId, ACCESS_WAKE_UP_EVT );
        }
    }
    if(pData[0] == RF_DATA_LED )
    {
        PRINT( "led\n" );
        if(last_led_data != pData[1])
        {
//            access_update_idel_sleep_timeout(IDEL_SLEEP_EVT_TIMEOUT);
            if(access_state.idel_sleep_flag)
            {
                // 停止睡眠
                access_state.sleep_en = FALSE;
                tmos_set_event( access_taskId, ACCESS_WAKE_UP_EVT );
            }
            last_led_data = pData[1];
            access_tran_report( REPORT_CMD_LED, pData[1] );
        }
    }
}


/*********************************************************************
 * @fn      access_tran_report
 *
 * @brief   上报状态
 *
 * @return  none
        peripheral_connecting_cb();
 */
void access_tran_report( uint8_t cmd, uint8_t data )
{
    if(data == STATE_CON_TERMINATE)
    {
        peripheral_disconnected_cb();
    }
    else if(data == STATE_CONNECTED)
    {
        peripheral_connected_cb();
    }
    else if(data == STATE_PAIRING)
    {
        peripheral_pairing_cb();
    }
    else if(data == STATE_RE_CONNECTING)
    {
        peripheral_connecting_cb();
    }
    else if(data == STATE_RE_CONNECT_FAIL)
    {
        peripheral_connecting_cb();
    }
    else if(cmd == REPORT_CMD_LED)
    {
        peripheral_pilot_led_receive(data);
    }
}

/*********************************************************************
 * @fn      access_switch_ble_mode
 *
 * @brief   1、之前也是BLE模式，则判断模式与当前模式是否相同，是则不做任何事，否则停止连接和广播，随后在状态处理里会开启或者等待配对命令
 *          2、之前是其他模式，则根据模式是否已绑定选择开始或者等待配对命令
 *
 * @return  none
 */
void access_switch_ble_mode( void )
{
    uint8_t ble_state;
//    uint8_t RL_enable = FALSE;
    if( nvs_flash_info.ble_idx != access_state.ble_idx)
    {
        nvs_flash_info.ble_idx = access_state.ble_idx;
        nvs_flash_store();
    }
//    ble.SNVAddr = (access_state.ble_idx - BLE_INDEX_2) * 0x100 + BLE_SNV_ADDR;
//    GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable );
//    RL_enable = TRUE;
//    GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable );
    if( (con_work_mode > BLE_INDEX_IDEL) && (con_work_mode < BLE_INDEX_MAX) )
    {
        if( con_work_mode != access_state.ble_idx )
        {
            access_state.pairing_state = FALSE;
            GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
            PRINT( "ble_state %x\n", ble_state );
            if( ble_state == GAPROLE_CONNECTED )
            {
                hidEmu_disconnect();
            }
            else if( ble_state == GAPROLE_ADVERTISING )
            {
                hidEmu_adv_enable( DISABLE );
            }
            else
            {
                if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
                {
                    access_tran_report( REPORT_CMD_STATE, STATE_RE_CONNECTING );
                    hidEmu_adv_enable( ENABLE );
                }
                else
                {
                    con_work_mode = access_state.ble_idx;
                    access_tran_report( REPORT_CMD_STATE, STATE_RE_CONNECT_FAIL );
                }
            }
            if( tmos_get_task_timer( hidEmuTaskId, SEND_DISCONNECT_EVT ) || (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT) )
            {
                tmos_stop_task(hidEmuTaskId, SEND_DISCONNECT_EVT);
                access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
            }
        }
        else //上次ble也是这个通道
        {
            access_state.pairing_state = FALSE;
            GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
            PRINT( "same ch ble_state %x\n", ble_state );
            if( (ble_state == GAPROLE_CONNECTED) || tmos_get_task_timer( hidEmuTaskId, SEND_DISCONNECT_EVT ))
            {
                access_tran_report( REPORT_CMD_STATE, STATE_CONNECTED );
                access_tran_report( REPORT_CMD_LED, last_led_data );
            }
            else if( ble_state == GAPROLE_ADVERTISING )
            {
            }
            else
            {
                if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
                {
                    access_tran_report( REPORT_CMD_STATE, STATE_RE_CONNECTING );
                    hidEmu_adv_enable( ENABLE );
                }
                else
                {
                    access_tran_report( REPORT_CMD_STATE, STATE_RE_CONNECT_FAIL );
                }
            }
        }
    }
    else
    {
        access_tran_report( REPORT_CMD_STATE, STATE_SWITCH_OVER );
        if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
        {
            access_tran_report( REPORT_CMD_STATE, STATE_RE_CONNECTING );
            hidEmu_adv_enable( ENABLE );
        }
        else
        {
            access_tran_report( REPORT_CMD_STATE, STATE_RE_CONNECT_FAIL );
        }
    }
}

/*********************************************************************
 * @fn      access_pairing_mode
 *
 * @brief   access_pairing_mode
 *
 * @return  none
 */
void access_pairing_mode( void )
{
    PRINT( "pairing_mode\n" );
    if( (access_state.ble_idx > BLE_INDEX_IDEL) && (access_state.ble_idx < BLE_INDEX_MAX) )
    {
        uint8_t ble_state;
        GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
        if( ble_state == GAPROLE_CONNECTED )
        {
            PRINT( "CON dis\n" );
            // 当前还在连接中，断开连接，换地址
            hidEmu_disconnect();
            access_state.pairing_state = TRUE;
            // 开启广播60s后进入睡眠，睡眠函数中如果还未连接，则停止广播直接睡眠
//            access_update_idel_sleep_timeout(ADV_IDEL_SLEEP_EVT_TIMEOUT);
            return;
        }
        if( hidEmu_is_ble_bonded( access_state.ble_idx ) )
        {
            // 当前已经绑定过换地址
            access_state.pairing_state = TRUE;
        }
        if( ble_state == GAPROLE_ADVERTISING )
        {
            if( con_work_mode == access_state.ble_idx )
            {
                // 同通道多次长按每次都要发STATE_PAIRING,不关广播
                PRINT( "same ch\n" );
                access_tran_report( REPORT_CMD_STATE, STATE_PAIRING );
            }
            hidEmu_adv_enable( DISABLE );
        }
        else
        {
            PRINT( "STATE_PAIRING\n" );
            access_tran_report( REPORT_CMD_STATE, STATE_PAIRING );
            hidEmu_adv_enable( ENABLE );
        }
        // 开启广播60s后进入睡眠，睡眠函数中如果还未连接，则停止广播直接睡眠
//        access_update_idel_sleep_timeout(ADV_IDEL_SLEEP_EVT_TIMEOUT);
    }
    else
    {
        PRINT( "ble_idx err %x\n", access_state.ble_idx );
    }
}

/*********************************************************************
 * @fn      access_pairing_process
 *
 * @brief   access_pairing_process
 *
 * @return  none
 */
void access_pairing_process( uint8_t ctl_type )
{
    switch( ctl_type )
    {
        case CTL_MODE_BLE_1:
        {
            PRINT( "BLE_1 %x\n" ,access_state.ble_idx);
            if( access_state.ble_idx == BLE_INDEX_1)
                access_pairing_mode();
            break;
        }
        case CTL_MODE_BLE_2:
        {
            if( access_state.ble_idx == BLE_INDEX_2)
                access_pairing_mode();
            break;
        }
        case CTL_MODE_BLE_3:
        {
            if( access_state.ble_idx == BLE_INDEX_3)
                access_pairing_mode();
            break;
        }
        case CTL_MODE_BLE_4:
        {
            if( access_state.ble_idx == BLE_INDEX_4)
                access_pairing_mode();
            break;
        }
        case CTL_MODE_BLE_5:
        {
            if( access_state.ble_idx == BLE_INDEX_5)
                access_pairing_mode();
            break;
        }
    }
}

/*********************************************************************
 * @fn      access_ctl_process
 *
 * @brief   access_ctl_process
 *
 * @return  none
 */
void access_ctl_process( uint8_t ctl_type )
{
    switch( ctl_type )
    {
        case CTL_MODE_BLE_1:
        {
            access_state.ble_idx = BLE_INDEX_1;
            access_switch_ble_mode();
            break;
        }
        case CTL_MODE_BLE_2:
        {
            access_state.ble_idx = BLE_INDEX_2;
            access_switch_ble_mode();
            break;
        }
        case CTL_MODE_BLE_3:
        {
            access_state.ble_idx = BLE_INDEX_3;
            access_switch_ble_mode();
            break;
        }
        case CTL_MODE_BLE_4:
        {
            access_state.ble_idx = BLE_INDEX_4;
            access_switch_ble_mode();
            break;
        }
        case CTL_MODE_BLE_5:
        {
            access_state.ble_idx = BLE_INDEX_5;
            access_switch_ble_mode();
            break;
        }
        case CTL_PAIRING:
        {
            access_pairing_mode();
            break;
        }
        case CTL_DELETE_PAIR_INFO:
        {
            PRINT( "CTL_DELETE_PAIR_INFO\n" );
            ble.SNVAddr = BLE_SNV_ADDR;
            GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, 0 );
            ble.SNVAddr = BLE_SNV_ADDR + 0x100;
            GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, 0 );
            ble.SNVAddr = BLE_SNV_ADDR + 0x200;
            GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, 0 );
            ble.SNVAddr = BLE_SNV_ADDR + 0x300;
            GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, 0 );
            ble.SNVAddr = BLE_SNV_ADDR + 0x400;
            GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, 0 );
            if( (access_state.ble_idx > BLE_INDEX_IDEL) && (access_state.ble_idx < BLE_INDEX_MAX) )
            {
                ble.SNVAddr = (access_state.ble_idx - BLE_INDEX_1) * 0x100 + BLE_SNV_ADDR;
            }
            nvs_flash_info.ble_bond_flag = 0;
            nvs_flash_info.ble_mac_flag = 0;
            nvs_flash_store();
            con_work_mode = BLE_INDEX_IDEL;
            hidEmu_disconnect();
            hidEmu_adv_enable( DISABLE );
            break;
        }
    }
}

/*********************************************************************
 * @fn      trans_send_data
 *
 * @brief
 *
 * @return  none
 */
uint8_t trans_send_data( uint8_t *pData, uint8_t len )
{
    return hidEmu_receive( pData, len );
}

void RF_ReStart( void )
{

}

/*********************************************************************
 * @fn      access_enter_idel_sleep
 *
 * @brief
 *
 * @return  none
 */
void access_enter_idel_sleep( void )
{
    tmos_set_event(access_taskId, ACCESS_IDEL_SLEEP_EVT);
}

/*********************************************************************
 * @fn      access_enter_deep_sleep
 *
 * @brief
 *
 * @return  none
 */
void access_enter_deep_sleep( void )
{
    tmos_set_event(access_taskId, ACCESS_SLEEP_EVT);
}

/*********************************************************************
 * @fn      access_weakup
 *
 * @brief
 *
 * @return  none
 */
__HIGH_CODE
void access_weakup( void )
{
    // 停止睡眠
    access_state.sleep_en = FALSE;
    tmos_set_event( access_taskId, ACCESS_WAKE_UP_EVT );
}

extern uint32_t __get_MEPC(void);
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void TMR1_IRQHandler( void )
{
    static uint8_t count=0;
//    if( TMR1_GetITFlag( TMR0_3_IT_CYC_END ) )
//    {
//        TMR1_ClearITFlag( TMR0_3_IT_CYC_END );
//        count++;
//        if( count>=4 )
//        {
//            uint32_t *data,*p;
//            uint32_t result;
//
//            __asm volatile ( "csrr %0," "mepc" : "=r" (result) );
//            while( 1 ){
//                UART0_SendByte(0x01);
//                UART0_SendByte(0x01);
//
//                UART0_SendByte(result&0xFF);
//                UART0_SendByte((result>>8)&0xFF);
//                UART0_SendByte((result>>16)&0xFF);
//                UART0_SendByte((result>>24)&0xFF);
//                UART0_SendByte(0x01);
//                UART0_SendByte(0x01);
//                DelayMs(10);
//            }
//            PRINT("\n\n");
//            printf("MEPC:%08x-%08x\n",__get_MEPC(),*(uint32_t*)(__get_MEPC()&0xfffffffc) );
//            asm volatile ("li a7, 0x20007800");
//            asm volatile ("sw sp, 0(a7)");
//            uint32_t *spp=(uint32_t *)0x20007800;
//            printf("spp: %x \n",*spp);
//            p = (uint32_t *)*spp;
//
//            __asm volatile ( "csrr %0," "mepc" : "=r" (result) );
//            PRINT("result %08x \r\n",result);
//            while( 1 ){
//              PRINT("%08x \r\n",*p);
//              p++;
//              if( (uint32_t)p > (uint32_t)(0x20008000) ) break;
//            }
//            PRINT("\n--------- ERROR ----%s--\n\n",__FUNCTION__ );
//
//            printf("\n--------%s--\n\n",__FUNCTION__ );
////            while(1);
//        }
//    }

}
/******************************** endfile  ******************************/
