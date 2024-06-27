/********************************** (C) COPYRIGHT *******************************
* File Name          : access.h
* Author             : tech7
* Version            : V1.0
* Date               : 2023/03/06
* Description        : 
            
*******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

#ifndef ACCESS_H
#define ACCESS_H

#ifndef SUCCESS
#define SUCCESS	0
#endif

#include "CONFIG.h"
#include "peripheral.h"

#define DISCONNECT_WAIT_REPORT_END_TIMEOUT       16*3
#define ADV_IDEL_SLEEP_EVT_TIMEOUT       1600*60*3
#define IDEL_SLEEP_EVT_TIMEOUT       1600*60*3
#define DISCONNECT_IDEL_SLEEP_EVT_TIMEOUT       1600*2  // 断开连接后多久停止广播睡眠
#define SLEEP_EVT_TIMEOUT       1600*60*30
#define BATT_EVT_TIMEOUT        1600*5
#define MODE_RECEIVE_EVT_TIMEOUT 2*1600
#define MODE_SWITCH_DELAY 1*160

#define RF_DATA_LED             CMD_CLASS_KEYBOARD
#define RF_DATA_IAP             0x92
#define RF_DATA_MANUFACTURER    CMD_MANUFACTURER
#define RF_DATA_SLEEP           0x93

#define LEN_CLASS_KEYBOARD      0x0A
#define LEN_ALL_KEYBOARD        0x10
#define LEN_CONSUMER            0x04
#define LEN_SYS_CTL             0x03
#define LEN_FN_DATA             0x03
#define LEN_CTL_TYPE            0x03
#define LEN_BATT_INFO           0x03
#define LEN_MOUSE               0x07
#define LEN_BLE_DEVICE_INFO     0x00
#define LEN_SMART_WHEEL         0x04
#define LEN_RESERVED            0x05
#define LEN_MANUFACTURER        0x22
#define LEN_SINGLE_CHANNEL      0x03

#define CTL_MODE_USB            0x11
#define CTL_MODE_2_4G           0x30
#define CTL_MODE_BLE_1          0x31
#define CTL_MODE_BLE_2          0x32
#define CTL_MODE_BLE_3          0x33
#define CTL_MODE_BLE_4          0x34
#define CTL_MODE_BLE_5          0x35
#define CTL_PAIRING             0x51
#define CTL_DELETE_PAIR_INFO    0x52
#define CTL_GET_BATT_INFO       0x53
#define CTL_ENABLE_BLE_SLEEP    0x55
#define CTL_DISABLE_BLE_SLEEP   0x56
#define CTL_ENABLE_2_4G_SLEEP   0x57
#define CTL_DISABLE_2_4G_SLEEP  0x58
#define CTL_ENTER_OTA           0x81
#define CTL_ENTER_TEST          0x82
#define CTL_A6_00_A6            0x00

#define REPORT_CMD_LED          0x5A
#define REPORT_CMD_STATE        0x5B
#define REPORT_CMD_BATT_INFO    0x5C
#define REPORT_CMD_MANUFACTURER 0x81

#define STATE_LOWPOWER_REPORT   0x21
#define STATE_LOWPOWER_CLOSED   0x22
#define STATE_POWER_RECOVER     0x23
#define STATE_PAIRING           0x31
#define STATE_CONNECTED         0x32
#define STATE_CON_TERMINATE     0x33
#define STATE_SWITCH_OVER       0x34
#define STATE_RE_CONNECTING     0x35
#define STATE_RE_CONNECT_FAIL   0x36
#define STATE_WAKE_UP           0x42
#define STATE_UART_CLOSED       0x43

// Task Events
#define ACCESS_SLEEP_EVT            1<<0
#define ACCESS_WAKE_UP_EVT          1<<2
#define ACCESS_IDEL_SLEEP_EVT       1<<4
#define ACCESS_SWITCH_BLE_MODE_EVT    1<<10
#define ACCESS_PAIRING_MODE_EVT    1<<11

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
    uint8_t pairing_state;
    uint8_t Fn_state;
    volatile uint8_t sleep_en;
    uint8_t deep_sleep_flag;
    uint8_t idel_sleep_flag;
    access_ble_idx_t  ble_idx;
}access_state_t;


typedef void (*opcodemapfun)(uint8_t * buff,uint8_t size);

 typedef struct
{
    uint8_t  cmd;
    uint8_t size;
}MCUtoRFcmd_t;


typedef struct
{
    uint8_t  cmd;
    opcodemapfun fun;
}MCUtoRFopcode_t;


/*********************************************************************
 * Global Variables
 */
extern access_state_t access_state;
extern uint8_t batt_val;
extern bleConfig_t ble;
extern uint8_t access_taskId;
extern uint8_t last_led_data;
extern uint8_t single_channel_flag;
/*********************************************************************
 * FUNCTIONS
 */
void access_init(void);

extern void access_ctl_process( uint8_t ctl_type );
void access_tran_report( uint8_t cmd, uint8_t data );
void access_update_idel_sleep_timeout( tmosTimer time );

__HIGH_CODE
bStatus_t tmos_set_event( tmosTaskID taskID, tmosEvents event );

extern tmosEvents tmos_get_event( tmosTaskID taskID );

/*********************************************************************
*********************************************************************/

#endif
