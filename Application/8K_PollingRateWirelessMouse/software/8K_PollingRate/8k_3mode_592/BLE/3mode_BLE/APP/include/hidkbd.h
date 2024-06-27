/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbd.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef HIDKBD_H
#define HIDKBD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "access.h"
/*********************************************************************
 * CONSTANTS
 */

// Task Events
#define START_DEVICE_EVT          0x0001
//#define START_REPORT_EVT          0x0002
#define START_PARAM_UPDATE_EVT    0x0004
#define START_PHY_UPDATE_EVT      0x0008
#define WAIT_TERMINATE_EVT          1<<5
#define SEND_PACKET_EVT          1<<6
#define DELETE_PACKET_EVT          1<<7
#define SEND_DISCONNECT_EVT          1<<8
#define BLE_CLEAR_BUF_EVT         1<<9
#define PERI_SECURITY_REQ_EVT         1<<10


/*********************************************************************
 * MACROS
 */
typedef void (*pfnHidEmuReceiveCB_t)( uint8_t *pData, uint8_t len );

// Minimum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    6

// Maximum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    10

// Slave latency to use if parameter update request
#define DEFAULT_DESIRED_SLAVE_LATENCY        30

// Supervision timeout value (units of 10ms)
#define DEFAULT_DESIRED_CONN_TIMEOUT         100

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t start_device_over;
extern uint16_t hidEmuConnHandle;
extern access_ble_idx_t con_work_mode;
extern uint8_t adv_enable_process_flag;
extern uint8_t hidEmuTaskId;
extern uint8_t hidDevConnSecure;
/*
 * Task Initialization for the BLE Application
 */
extern void HidEmu_Init(void);

/*
 * Task Event Processor for the BLE Application
 */
extern uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events);

extern void hidEmu_adv_enable(uint8_t enable);

extern uint8_t hidEmu_is_ble_bonded( access_ble_idx_t ble_idx );

extern uint8_t hidEmu_is_ble_mac_change( access_ble_idx_t ble_idx );

extern uint8_t hidEmu_update_device_name(void);

extern uint8_t hidEmu_class_keyboard_report(uint8_t *pData,uint8_t len);

extern uint8_t hidEmu_mouse_report(uint8_t *pData,uint8_t len);

extern uint8_t hidEmu_all_keyboard_report(uint8_t *pData,uint8_t len);

extern uint8_t hidEmu_fn_report(uint8_t *pData, uint8_t len);

extern uint8_t hidEmu_consumer_report(uint8_t *pData,uint8_t len);

extern uint8_t hidEmu_sys_ctl_report(uint8_t data);

extern uint8_t hidEmu_smart_wheel_report(uint8_t *pData, uint8_t len);

extern void hidEmu_receive_cb_register(pfnHidEmuReceiveCB_t cback);

extern void hidEmu_save_ble_bonded(uint8_t is_pairing);

extern void hidEmu_delete_ble_bonded(void);

extern void hidEmu_disconnect(void);

extern uint8_t hidEmu_receive( uint8_t *pData, uint8_t len );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
