/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbd.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 蓝牙键盘应用程序，初始化广播连接参数，然后广播，直至连接主机后，定时上传键值
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "mouse.h"
#include "battservice.h"
#include "hidkbdservice.h"
#include "access.h"
#include "hiddev.h"
#include "hidkbd.h"
#include "HAL.h"

/*********************************************************************
 * MACROS
 */
// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN              8

// HID LED output report length
#define HID_LED_OUT_RPT_LEN                  1

/*********************************************************************
 * CONSTANTS
 */
#define TERMINATE_EVT_TIMEOUT   160


// PHY update delay
#define START_PHY_UPDATE_DELAY               1600

// HID idle timeout in msec; set to zero to disable timeout
#define DEFAULT_HID_IDLE_TIMEOUT             60000

// Default passcode
#define DEFAULT_PASSCODE                     0

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE                 GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE                    FALSE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE                 TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES              GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL          6

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Task ID
uint8_t hidEmuTaskId = INVALID_TASK_ID;

uint8_t start_device_over = FALSE;
uint8_t adv_enable_process_flag = FALSE;
/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// GAP Profile - Name attribute for SCAN RSP data
static uint8_t scanRspData[31] = {

    0x0D,                           // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE, // AD Type = Complete local name
    'H',
    'I',
    'D',
    ' ',
    'K',
    'e',
    'y',
    'b',
    'r',
    'o',
    'a',
    'd'  // connection interval range

};

// Advertising data
static uint8_t advertData[31] = {
    // flags
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // service UUIDs
    0x03, // length of this data
    GAP_ADTYPE_16BIT_MORE,
    LO_UINT16(HID_SERV_UUID),
    HI_UINT16(HID_SERV_UUID),
//    LO_UINT16(BATT_SERV_UUID),
//    HI_UINT16(BATT_SERV_UUID),

    // appearance
    0x03, // length of this data
    GAP_ADTYPE_APPEARANCE,
    LO_UINT16(GAP_APPEARE_HID_KEYBOARD),
    HI_UINT16(GAP_APPEARE_HID_KEYBOARD)
};

// Advertising data
static uint8_t reconAdvertData[1] = {
    0
//    // flags
//    0x02, // length of this data
//    GAP_ADTYPE_FLAGS,
//    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED

};

// HID Dev configuration
static hidDevCfg_t hidEmuCfg = {
    DEFAULT_HID_IDLE_TIMEOUT, // Idle timeout
    HID_FEATURE_FLAGS         // HID feature flags
};

uint16_t hidEmuConnHandle = GAP_CONNHANDLE_INIT;

access_ble_idx_t con_work_mode = BLE_INDEX_IDEL;

#define BLE_SEND_BUF_LEN      20   //  缓存5个包

typedef struct
{
    uint8_t len;
    uint8_t resend;
    uint8_t pData[20];
}BLE_send_buffer_t;

BLE_send_buffer_t BLE_send_buffer[BLE_SEND_BUF_LEN]={0};

uint8_t BLE_buf_out_idx=0;
uint8_t BLE_buf_data_num=0;
uint8_t BLE_buf_resend_num=0;
/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void    hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData);
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData);
static void    hidEmuEvtCB(uint8_t evt);
static void    hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);

uint8_t HCI_MB_DisconnectCmd( uint16_t connHandle, uint8_t reason );
extern uint8_t LL_SetDataRelatedAddressChanges( uint8_t Advertising_Handle, uint8_t Change_Reasons ) ;
void hidEmu_NEXT_BUF(void);

/*********************************************************************
 * PROFILE CALLBACKS
 */

static hidDevCB_t hidEmuHidCBs = {
    hidEmuRptCB,
    hidEmuEvtCB,
    NULL,
    hidEmuStateCB};

pfnHidEmuReceiveCB_t    hidEmu_receive_cb = 0;

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      HidEmu_Init
 *
 * @brief   Initialization function for the HidEmuKbd App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void HidEmu_Init()
{
    hidEmuTaskId = TMOS_ProcessEventRegister(HidEmu_ProcessEvent);

    // Setup the GAP Peripheral Role Profile
    {
        uint8_t initial_advertising_enable = FALSE;
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, 32);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, 32);

        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);

        // 从flash回复设备名称
        scanRspData[0] = nvs_flash_info.ble_name_len+1;
        scanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
        tmos_memcpy(&scanRspData[2], nvs_flash_info.ble_name_data, nvs_flash_info.ble_name_len);
        tmos_memcpy(&advertData[13-2], scanRspData, nvs_flash_info.ble_name_len+2);
        PRINT("len %d %x\n",scanRspData[0],scanRspData[1]);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, nvs_flash_info.ble_name_len+2+13-2, advertData);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, nvs_flash_info.ble_name_len+2, scanRspData);
    }

    // Set the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, nvs_flash_info.ble_name_len, (void *)nvs_flash_info.ble_name_data);

    // Setup the GAP Bond Manager
    {
        uint32_t passkey = DEFAULT_PASSCODE;
        uint8_t  pairMode = DEFAULT_PAIRING_MODE;
        uint8_t  mitm = DEFAULT_MITM_MODE;
        uint8_t  ioCap = DEFAULT_IO_CAPABILITIES;
        uint8_t  bonding = DEFAULT_BONDING_MODE;
//        uint8_t  RL_enable = ENABLE;
        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
//        GAPBondMgr_SetParameter(GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable);
    }

    // Setup Battery Characteristic Values
    {
        uint8_t critical = DEFAULT_BATT_CRITICAL_LEVEL;
        Batt_SetParameter(BATT_PARAM_CRITICAL_LEVEL, sizeof(uint8_t), &critical);
    }

    // Set up HID keyboard service
    Hid_AddService();

    // Register for HID Dev callback
    HidDev_Register(&hidEmuCfg, &hidEmuHidCBs);

    // Setup a delayed profile startup
    tmos_set_event(hidEmuTaskId, START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      HidEmu_ProcessEvent
 *
 * @brief   HidEmuKbd Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(hidEmuTaskId)) != NULL)
        {
            hidEmu_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);

            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & START_DEVICE_EVT)
    {
        return (events ^ START_DEVICE_EVT);
    }


    if(events & START_PHY_UPDATE_EVT)
    {
        // start phy update
        PRINT("Send Phy Update %x...\n", GAPRole_UpdatePHY(hidEmuConnHandle, 0, 
                    GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));

        return (events ^ START_PHY_UPDATE_EVT);
    }

    if(events & PERI_SECURITY_REQ_EVT)
    {
        // start phy update
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            PRINT("Send Security Req ...\n");
            if(GAPBondMgr_PeriSecurityReq(hidEmuConnHandle))
            {
                tmos_start_task(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);
            }
        }

        return (events ^ PERI_SECURITY_REQ_EVT);
    }

    if(events & WAIT_TERMINATE_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            HCI_MB_DisconnectCmd(hidEmuConnHandle, 0x16);
        }
        return (events ^ WAIT_TERMINATE_EVT);
    }

    if(events & SEND_DISCONNECT_EVT)
    {
        PRINT("SEND_DISCONNECT_EVT\n");
        uint8_t ble_state;
//        PRINT("SD\n");
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state != GAPROLE_CONNECTED)
        {
            access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
            last_led_data = 0xFF;
            // 清空buff
            BLE_buf_out_idx=0;
            BLE_buf_data_num=0;
            BLE_buf_resend_num=0;
        }
        return (events ^ SEND_DISCONNECT_EVT);
    }

    if(events & SEND_PACKET_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            if(BLE_buf_data_num && (BLE_send_buffer[BLE_buf_out_idx].resend==0))
            {
                uint8_t state=0;
                switch(BLE_send_buffer[BLE_buf_out_idx].pData[0])
                {
                    case CMD_CLASS_KEYBOARD:
                        state = hidEmu_class_keyboard_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
                        break;

                    case CMD_ALL_KEYBOARD:
                        state = hidEmu_all_keyboard_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
                        break;

                    case CMD_CONSUMER:
                        state = hidEmu_consumer_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
                        break;

                    case CMD_SYS_CTL:
                        state = hidEmu_sys_ctl_report(BLE_send_buffer[BLE_buf_out_idx].pData[1]);
                        break;

                    case CMD_MOUSE:
                        state = hidEmu_mouse_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
                        break;

                    case CMD_FN_DATA:
                        state = hidEmu_fn_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
                        break;
//
//                    case CMD_SMART_WHEEL:
//                        state = hidEmu_smart_wheel_report(&BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].len-1);
//                        break;

                    default:
                    {
//                        uint8_t temp1;
//                        uint8_t temp2;
                        PRINT("code %x\n",BLE_send_buffer[BLE_buf_out_idx].pData[0]);
//                        PRINT("buf err$\n");
//                        temp1 = BLE_buf_data_num;
//                        temp2 = BLE_buf_out_idx;
//                        while(temp1)
//                        {
//                            PRINT("buf %d %d: %x %x %x %x %x\n",temp2,BLE_send_buffer[temp2].len,BLE_send_buffer[temp2].pData[0],BLE_send_buffer[temp2].pData[1],BLE_send_buffer[temp2].pData[2]
//                                                       ,BLE_send_buffer[temp2].pData[3],BLE_send_buffer[temp2].pData[4]);
//                            (temp2==(BLE_SEND_BUF_LEN-1))?(temp2=0):(temp2++);
//                            temp1--;
//                        }
//                        // 清空buff
//                        BLE_buf_out_idx=0;
//                        BLE_buf_data_num=0;
//                        BLE_buf_resend_num=0;
                        state = 0xF1;
                        break;
                    }
                }
                if( (!state) || (state == 0xF1))
                {
//                    PRINT("1\n");
                    BLE_send_buffer[BLE_buf_out_idx].resend = 1;
                    BLE_buf_resend_num++;
//                    PRINT("b %d %d: %x %x %x %x %x\n",BLE_buf_out_idx,BLE_send_buffer[BLE_buf_out_idx].len,BLE_send_buffer[BLE_buf_out_idx].pData[0],BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].pData[2]
//                                               ,BLE_send_buffer[BLE_buf_out_idx].pData[3],BLE_send_buffer[BLE_buf_out_idx].pData[4]);
//                    {
                        (BLE_buf_out_idx==(BLE_SEND_BUF_LEN-1))?(BLE_buf_out_idx=0):(BLE_buf_out_idx++);
                        if((BLE_buf_data_num - BLE_buf_resend_num) >0)
                        {
                            tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
                        }
//                    }
//                    else {
//                        PRINT("all resend\n");
//                        (BLE_buf_out_idx==(BLE_SEND_BUF_LEN-1))?(BLE_buf_out_idx=0):(BLE_buf_out_idx++);
//                        tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
//                    }
                }
                else {
                    PRINT("!\n");
                    tmos_start_task(hidEmuTaskId, SEND_PACKET_EVT, 4);
                }
                tmos_start_task(hidEmuTaskId, DELETE_PACKET_EVT, 4);
            }
        }
        return (events ^ SEND_PACKET_EVT);
    }

    if(events & DELETE_PACKET_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state == GAPROLE_CONNECTED)
        {
            uint8_t UnAckPacket = LL_GetNumberOfUnAckPacket(hidEmuConnHandle);
            if( UnAckPacket < BLE_buf_resend_num )
            {
                BLE_buf_data_num -= (BLE_buf_resend_num - UnAckPacket);
                BLE_buf_resend_num = UnAckPacket;
            }
            if(UnAckPacket)
            {
                tmos_start_task(hidEmuTaskId, DELETE_PACKET_EVT, 4);
            }
        }
        return (events ^ DELETE_PACKET_EVT);
    }

    if(events & BLE_CLEAR_BUF_EVT)
    {
        uint8_t ble_state;
        GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
        if(ble_state != GAPROLE_CONNECTED)
        {
            // 清空buff
            BLE_buf_out_idx=0;
            BLE_buf_data_num=0;
            BLE_buf_resend_num=0;
        }
        return (events ^ BLE_CLEAR_BUF_EVT);
    }
    return 0;
}

/*********************************************************************
 * @fn      hidEmu_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        default:
            break;
    }
}
/*********************************************************************
 * @fn      hidEmu_disconnect
 *
 * @brief   强制断开连接
 *
 * @return  none.
 */
void hidEmu_disconnect()
{
    GAPRole_TerminateLink(hidEmuConnHandle);
    if( tmos_get_task_timer( hidEmuTaskId, WAIT_TERMINATE_EVT ) || (tmos_get_event(hidEmuTaskId)&WAIT_TERMINATE_EVT) )
    {

    }
    else
    {
        tmos_start_task( hidEmuTaskId , WAIT_TERMINATE_EVT, TERMINATE_EVT_TIMEOUT );
    }
}

/*********************************************************************
 * @fn      hidEmu_delete_ble_bonded
 *
 * @brief   清楚当前模式的设备的绑定标志，存储相关绑定标志信息到flash
 *
 * @return  none.
 */
void hidEmu_delete_ble_bonded()
{
    switch(con_work_mode)
    {
        case BLE_INDEX_1:
            nvs_flash_info.ble_bond_flag &= ~BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            nvs_flash_info.ble_bond_flag &= ~BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            nvs_flash_info.ble_bond_flag &= ~BLE_BOND_FLAG_3;
            break;
        case BLE_INDEX_4:
            nvs_flash_info.ble_bond_flag &= ~BLE_BOND_FLAG_4;
            break;
        case BLE_INDEX_5:
            nvs_flash_info.ble_bond_flag &= ~BLE_BOND_FLAG_5;
            break;
        default:
            PRINT("work mode err %x\n",con_work_mode);
            return;
            break;
    }
    nvs_flash_store();
}

/*********************************************************************
 * @fn      hidEmu_save_ble_bonded
 *
 * @brief   存储相关绑定标志信息到flash，自动调用此函数，调用前已经将绑定信息存到flash中
 *
 * @return  none.
 */
void hidEmu_save_ble_bonded(uint8_t is_pairing)
{
    switch(con_work_mode)
    {
        case BLE_INDEX_1:
            nvs_flash_info.ble_bond_flag |= BLE_BOND_FLAG_1;
            if(is_pairing)
            {
                nvs_flash_info.ble_mac_flag ^= BLE_BOND_FLAG_1;
            }
            break;
        case BLE_INDEX_2:
            nvs_flash_info.ble_bond_flag |= BLE_BOND_FLAG_2;
            if(is_pairing)
            {
                nvs_flash_info.ble_mac_flag ^= BLE_BOND_FLAG_2;
            }
            break;
        case BLE_INDEX_3:
            nvs_flash_info.ble_bond_flag |= BLE_BOND_FLAG_3;
            if(is_pairing)
            {
                nvs_flash_info.ble_mac_flag ^= BLE_BOND_FLAG_3;
            }
            break;
        case BLE_INDEX_4:
            nvs_flash_info.ble_bond_flag |= BLE_BOND_FLAG_4;
            if(is_pairing)
            {
                nvs_flash_info.ble_mac_flag ^= BLE_BOND_FLAG_4;
            }
            break;
        case BLE_INDEX_5:
            nvs_flash_info.ble_bond_flag |= BLE_BOND_FLAG_5;
            if(is_pairing)
            {
                nvs_flash_info.ble_mac_flag ^= BLE_BOND_FLAG_5;
            }
            break;
        default:
            PRINT("work mode err %x\n",con_work_mode);
            return;
            break;
    }
    nvs_flash_store();
}

/*********************************************************************
 * @fn      hidEmu_is_ble_mac_change
 *
 * @brief   判断mac地址是否需要+1
 *
 * @return  none.
 */
uint8_t hidEmu_is_ble_mac_change( access_ble_idx_t ble_idx )
{
    switch(ble_idx)
    {
        case BLE_INDEX_1:
            return nvs_flash_info.ble_mac_flag & BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            return nvs_flash_info.ble_mac_flag & BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            return nvs_flash_info.ble_mac_flag & BLE_BOND_FLAG_3;
            break;
        case BLE_INDEX_4:
            return nvs_flash_info.ble_mac_flag & BLE_BOND_FLAG_4;
            break;
        case BLE_INDEX_5:
            return nvs_flash_info.ble_mac_flag & BLE_BOND_FLAG_5;
            break;
        default:
            PRINT("work mode err %x\n",ble_idx);
            return 0;
            break;
    }
}

/*********************************************************************
 * @fn      hidEmu_is_ble_bonded
 *
 * @brief   判断是否绑定
 *
 * @return  none.
 */
uint8_t hidEmu_is_ble_bonded( access_ble_idx_t ble_idx )
{
    PRINT("ble_bond_flag %x T %x\n",nvs_flash_info.ble_bond_flag,ble_idx);
    switch(ble_idx)
    {
        case BLE_INDEX_1:
            return nvs_flash_info.ble_bond_flag & BLE_BOND_FLAG_1;
            break;
        case BLE_INDEX_2:
            return nvs_flash_info.ble_bond_flag & BLE_BOND_FLAG_2;
            break;
        case BLE_INDEX_3:
            return nvs_flash_info.ble_bond_flag & BLE_BOND_FLAG_3;
            break;
        case BLE_INDEX_4:
            return nvs_flash_info.ble_bond_flag & BLE_BOND_FLAG_4;
            break;
        case BLE_INDEX_5:
            return nvs_flash_info.ble_bond_flag & BLE_BOND_FLAG_5;
            break;
        default:
            PRINT("no bond mode %x\n",ble_idx);
            return 0;
            break;
    }
}

/*********************************************************************
 * @fn      hidEmu_adv_enable
 *
 * @brief   打开广播，并根据是否绑定选择开始过滤的广播，更换广播名称.
 *
 * @return  none.
 */
void hidEmu_adv_enable(uint8_t enable)
{
    uint8_t i,need_update=0;
    uint8_t ownAddr[6];
    uint8_t initial_advertising_enable = enable;
    uint8_t IRK[KEYLEN]={0};

    if(initial_advertising_enable)
    {
        uint8_t advertising_state;
        uint8_t RL_enable = TRUE;
        GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &advertising_state );
        PRINT("adv state %x\n",advertising_state);
        if( !advertising_state )
        {
            adv_enable_process_flag = TRUE;
        }
        ble.SNVAddr = (access_state.ble_idx - BLE_INDEX_1) * 0x100 + BLE_SNV_ADDR;
        GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable );
        RL_enable = FALSE;
        GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_RL, sizeof(uint8_t), &RL_enable );

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddr);
        // 修改广播地址。
        tmos_snv_read(BLE_NVID_IRK,KEYLEN,IRK);
        PRINT("IRK %x %x %x %x %x %x\n",IRK[5],IRK[4],IRK[3],IRK[2],IRK[1],IRK[0]);
        GAPRole_SetParameter(GAPROLE_IRK, KEYLEN, IRK);
        ownAddr[4] += access_state.ble_idx;
        if( access_state.pairing_state )
        {
            // 如果更换标志未置位，则更换
            if(!hidEmu_is_ble_mac_change(access_state.ble_idx))
            {
                ownAddr[3] += access_state.ble_idx;
            }
        }
        else
        {
            // 如果更换标志已置位，则更换
            if(hidEmu_is_ble_mac_change(access_state.ble_idx))
            {
                ownAddr[3] += access_state.ble_idx;
            }
        }
        PRINT("%x %x %x %x %x %x\n",ownAddr[5],ownAddr[4],ownAddr[3],ownAddr[2],ownAddr[1],ownAddr[0]);
        GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, ownAddr);

        // 有需求是不同通道蓝牙名字不一样比如通道1名称为“BT-1”，通道2名称为“BT-2”等，则MCU发送是“BT-$”,这个美元符号表示蓝牙不同通道显示不同的名称
        for(i=0; i<nvs_flash_info.ble_name_len; i++ )
        {
            if(nvs_flash_info.ble_name_data[i]=='$')
            {
                scanRspData[2+i] = access_state.ble_idx+0x30-BLE_INDEX_IDEL;
                need_update = 1;
            }
        }
        if( need_update )
        {
            PRINT( "need_update\n");
            GGS_SetParameter(GGS_DEVICE_NAME_ATT, nvs_flash_info.ble_name_len, (void *)&scanRspData[2]);
        }
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, nvs_flash_info.ble_name_len+2, scanRspData);
        tmos_memcpy(&advertData[13-2], scanRspData, nvs_flash_info.ble_name_len+2);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, nvs_flash_info.ble_name_len+2+13-2, advertData);
    }
    if(initial_advertising_enable && hidEmu_is_ble_bonded(access_state.ble_idx) && (!access_state.pairing_state))
    {
//        uint8_t filter_policy = GAP_FILTER_POLICY_WHITE;
        uint8_t filter_policy = GAP_FILTER_POLICY_WHITE_SCAN;
        PRINT("WHITE\n");
        GAPRole_SetParameter(GAPROLE_ADV_FILTER_POLICY, sizeof(uint8_t), &filter_policy);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(reconAdvertData), reconAdvertData);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(reconAdvertData), reconAdvertData);
//        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, 0, reconAdvertData);
    }
    else
    {
        PRINT("disable adv\n");
        uint8_t filter_policy = GAP_FILTER_POLICY_ALL;
        GAPRole_SetParameter(GAPROLE_ADV_FILTER_POLICY, sizeof(uint8_t), &filter_policy);
    }
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
}

/*********************************************************************
 * @fn      hidEmu_update_adv_data
 *
 * @brief   HID Dev update_adv_data.
 *
 * @return  none.
 */
uint8_t hidEmu_update_device_name()
{
    uint8_t i;
    scanRspData[0] = nvs_flash_info.ble_name_len+1;
    scanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
    tmos_memcpy(&scanRspData[2], nvs_flash_info.ble_name_data, nvs_flash_info.ble_name_len);
    for(i=0; i<nvs_flash_info.ble_name_len; i++ )
    {
        if(nvs_flash_info.ble_name_data[i]=='$')
        {
            scanRspData[2+i] = access_state.ble_idx+0x30-BLE_INDEX_IDEL;
//                need_update = 1;
        }
    }
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, nvs_flash_info.ble_name_len, (void *)&scanRspData[2]);
    return GAP_UpdateAdvertisingData( hidEmuTaskId, FALSE, nvs_flash_info.ble_name_len+2, scanRspData );
}

/*********************************************************************
 * @fn      hidEmu_resend_BUF
 *
 * @brief   hidEmu_resend_BUF
 *
 * @return  none
 */
void hidEmu_resend_BUF(void)
{
    while(BLE_buf_resend_num)
    {
        (BLE_buf_out_idx==0)?(BLE_buf_out_idx=(BLE_SEND_BUF_LEN-1)):(BLE_buf_out_idx--);
        BLE_send_buffer[BLE_buf_out_idx].resend = 0;
        BLE_buf_resend_num--;
//        PRINT("re %d %d: %x %x %x\n",BLE_buf_out_idx,BLE_send_buffer[BLE_buf_out_idx].len,BLE_send_buffer[BLE_buf_out_idx].pData[0]
//                   ,BLE_send_buffer[BLE_buf_out_idx].pData[1],BLE_send_buffer[BLE_buf_out_idx].pData[2]);
    }
}

/*********************************************************************
 * @fn      hidEmu_receive
 *
 * @brief   hidEmu_receive
 *
 * @return  none
 */
uint8_t hidEmu_receive( uint8_t *pData, uint8_t len )
{
    uint8_t input_idx;
    uint8_t ble_state;
    GAPRole_GetParameter(GAPROLE_STATE, &ble_state);
    if((ble_state != GAPROLE_CONNECTED)&&
        (tmos_get_task_timer(hidEmuTaskId, BLE_CLEAR_BUF_EVT)==0)&&
        ((tmos_get_event(hidEmuTaskId)&BLE_CLEAR_BUF_EVT)==0))
    {
        tmos_start_task(hidEmuTaskId, BLE_CLEAR_BUF_EVT, 1600*5);
    }
    if(BLE_buf_data_num==BLE_SEND_BUF_LEN)
    {
        PRINT("BLE_BUF FF\n");
        tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
        return 0xFF;
    }
    input_idx = (BLE_buf_out_idx+BLE_buf_data_num-BLE_buf_resend_num)%BLE_SEND_BUF_LEN;
    BLE_send_buffer[input_idx].len = len;
    BLE_send_buffer[input_idx].resend = 0;
    tmos_memcpy((BLE_send_buffer[input_idx].pData), pData, len);
    BLE_buf_data_num++;
    tmos_set_event(hidEmuTaskId, SEND_PACKET_EVT);
    return 0;

//    PRINT("RF %x %d\n",pData[0], len);
}

/*********************************************************************
 * @fn      hidEmu_class_keyboard_report
 *
 * @brief   Build and send a HID report.
 *
 * @return  none
 */
uint8_t hidEmu_class_keyboard_report(uint8_t *pData, uint8_t len)
{
    return HidDev_Report(HID_RPT_ID_CLASS_KEY_IN, HID_REPORT_TYPE_INPUT, len, pData);
}

/*********************************************************************
 * @fn      hidEmu_mouse_report
 *
 * @brief   Build and send a HID report.
 *
 * @return  none
 */
uint8_t hidEmu_mouse_report(uint8_t *pData, uint8_t len)
{
    return HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, len, pData);
}

/*********************************************************************
 * @fn      hidEmu_consumer_report
 *
 * @brief   Build and send a HID report.
 *
 * @return  none
 */
uint8_t hidEmu_consumer_report(uint8_t *pData, uint8_t len)
{
    return HidDev_Report(HID_RPT_ID_CONSUMER_IN, HID_REPORT_TYPE_INPUT, len, pData);
}

/*********************************************************************
 * @fn      hidEmu_sys_ctl_report
 *
 * @brief   Build and send a HID report.
 *
 * @return  none
 */
uint8_t hidEmu_sys_ctl_report(uint8_t data)
{
    return HidDev_Report(HID_RPT_ID_SYS_CTL_IN, HID_REPORT_TYPE_INPUT, 1, &data);
}

/*********************************************************************
 * @fn      hidEmu_all_keyboard_report
 *
 * @brief   Build and send a HID report.
 *
 * @return  none
 */
uint8_t hidEmu_all_keyboard_report(uint8_t *pData, uint8_t len)
{
    return HidDev_Report(HID_RPT_ID_ALL_KEY_IN, HID_REPORT_TYPE_INPUT, len, pData);
}

/*********************************************************************
 * @fn      hidEmu_fn_report
 *
 * @brief   Build and send a HID report.
 *
 * @return  none
 */
uint8_t hidEmu_fn_report(uint8_t *pData, uint8_t len)
{
    return HidDev_Report(HID_RPT_ID_FN_IN, HID_REPORT_TYPE_INPUT, len, pData);
}

/*********************************************************************
 * @fn      hidEmu_smart_wheel_report
 *
 * @brief   Build and send a HID report.
 *
 * @return  none
 */
uint8_t hidEmu_smart_wheel_report(uint8_t *pData, uint8_t len)
{
    return HidDev_Report(HID_RPT_ID_SMART_WHEEL_IN, HID_REPORT_TYPE_INPUT, len, pData);
}

/*********************************************************************
 * @fn      hidEmuStateCB
 *
 * @brief   GAP state change callback.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState & GAPROLE_STATE_ADV_MASK)
    {
        case GAPROLE_STARTED:
        {
            start_device_over = TRUE;
            LL_SetDataRelatedAddressChanges( 1, 1 ) ;
            if(access_taskId != INVALID_TASK_ID)
            {
                // 初始化完成后，检查是否需要直接切换到对应连接
                if(nvs_flash_info.ble_idx != access_state.ble_idx)
                {
                    access_ctl_process( nvs_flash_info.ble_idx + CTL_MODE_BLE_1 - BLE_INDEX_1);
                }
            }
            PRINT("Initialized..\n");
        }
        break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                adv_enable_process_flag = FALSE;
                // 记下当前模式
                if((access_state.ble_idx<BLE_INDEX_1) || (access_state.ble_idx>BLE_INDEX_5))
                {
                    PRINT("ADV mode err.. %x\n",access_state.ble_idx);
                    // 状态错误，关闭广播
                    hidEmu_adv_enable(DISABLE);
                }
                else
                {
                    con_work_mode = access_state.ble_idx;
                }
                if( tmos_get_task_timer( hidEmuTaskId, SEND_DISCONNECT_EVT ) || (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT) )
                {
                }
                else {
                    last_led_data = 0xFF;
                }
                PRINT("Advertising..\n");
            }
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

                // get connection handle
                hidEmuConnHandle = event->connectionHandle;

                tmos_start_task(hidEmuTaskId, PERI_SECURITY_REQ_EVT, 4800);
                tmos_stop_task(hidEmuTaskId, BLE_CLEAR_BUF_EVT);
                if( tmos_get_task_timer( hidEmuTaskId, SEND_DISCONNECT_EVT ) || (tmos_get_event(hidEmuTaskId)&SEND_DISCONNECT_EVT) )
                {
                  PRINT("buf_num %d\n",BLE_buf_data_num);
                }
                else {
                  // 清空buff
                  BLE_buf_out_idx=0;
                  BLE_buf_data_num=0;
                  BLE_buf_resend_num=0;
                }
                PRINT("Connected..int %x\n",event->connInterval);
//                {
//                    uint32_t time;
//                    time = RTC_GetCycle32k();
//                    time += WAKE_UP_RTC_MAX_TIME*10;
//                    if(time > 0xA8C00000)
//                    {
//                        time -= 0xA8C00000;
//                    }
//                    RTC_SetTignTime(time);
//                    // LOW POWER-sleep模式
//                    if((!RTCTigFlag)&&(GPIOB_ReadPortPin(bRXD1_)))
//                    {
//                        LowPower_Sleep(RB_PWR_RAM2K | RB_PWR_RAM30K | RB_PWR_EXTEND);
//                //        LowPower_Idle();
//                        GPIOA_ResetBits(bTXD0_);
//                        mDelaymS(1);
//                        BLE_RegInit();
//                        GPIOA_SetBits(bTXD0_);
//                        HSECFG_Current(HSE_RCur_100); // 降为额定电流(低功耗函数中提升了HSE偏置电流)
//                    }
//                }
//                access_update_idel_sleep_timeout(0);
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Connected Advertising..\n");
            }
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                // 1、切换到其他模式，命令停止的广播，则判断模式，不开启新的广播
                // 2、还是当前模式没变，只是limit广播自动停止，则继续广播(注意是否是OTA模式)
                // 3、切换到另外的蓝牙模式，命令停止的广播，则修改mac地址，判断是否已经绑定过，是则开启广播，并开启过滤，否则不开启广播，等待配对命令。
                PRINT("con_mode %x\n",con_work_mode);
                if((con_work_mode == access_state.ble_idx))
                {
                    if( access_state.ble_idx == BLE_INDEX_MAX)
                    {
                        uint8_t initial_advertising_enable = ENABLE;
                        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
                    }
                    else
                    {
                        // limit广播自动停止，重新打开即可
                        hidEmu_adv_enable(ENABLE);
                    }
                }
                else if((access_state.ble_idx>BLE_INDEX_IDEL) && (access_state.ble_idx<BLE_INDEX_MAX) )
                {
                    if(hidEmu_is_ble_bonded(access_state.ble_idx))
                    {
                        if( access_state.pairing_state )
                        {
                            access_tran_report(REPORT_CMD_STATE, STATE_PAIRING);
                        }
                        else
                        {
                            // 进入新蓝牙模式的回连状态
                            access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECTING);
                        }
                        hidEmu_adv_enable(ENABLE);
                    }
                    else {
                        // 没绑定过，无法回连
                        con_work_mode = access_state.ble_idx;
                        access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECT_FAIL);
                    }
                }
//                // 记下当前模式 睡眠后模式改为idel，所以这里不能同步模式
//                con_work_mode = access_state.ble_idx;
                PRINT("Waiting for advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                if(tmos_get_task_timer(hidEmuTaskId, WAIT_TERMINATE_EVT))
                {
                    tmos_stop_task(hidEmuTaskId, WAIT_TERMINATE_EVT);
                }
//                // 上报蓝牙断开
//                // 如果当前已经是2.4G模式的话，说明已经发过的断开，不再发。
//                if( access_state.ble_idx != WORK_MODE_2_4G)
//                {
//                    access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
//                }
                // 1、切换到其他模式，命令停止的连接，则判断模式，不开启新的广播
                // 2、还是当前模式没变，只是连接断开，则继续广播，并开启过滤,(注意是否是OTA模式)
                // 3、切换到另外的蓝牙模式，命令停止的连接，则修改mac地址，判断是否已经绑定过，是则开启广播，并开启过滤，否则不开启广播，等待配对命令。
                if((con_work_mode == access_state.ble_idx) ||
                    ( (access_state.ble_idx>BLE_INDEX_IDEL) && (access_state.ble_idx<BLE_INDEX_MAX) && ( hidEmu_is_ble_bonded(access_state.ble_idx) ) ))
                {
                    if( access_state.ble_idx == BLE_INDEX_MAX)
                    {
                        uint8_t initial_advertising_enable = ENABLE;
                        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
                        // 上报蓝牙断开
                        // 如果当前已经是2.4G模式的话，说明已经发过的断开，不再发。
                        if( access_state.ble_idx != BLE_INDEX_IDEL)
                        {
                            access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
                        }
                    }
                    else
                    {
                        if( access_state.pairing_state )
                        {
                            // 上报蓝牙断开
                            // 如果当前已经是2.4G模式的话，说明已经发过的断开，不再发。
                            if( access_state.ble_idx != BLE_INDEX_IDEL)
                            {
                                access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
                            }
                            access_tran_report(REPORT_CMD_STATE, STATE_PAIRING);
                            hidEmu_adv_enable(ENABLE);
                        }
                        else
                        {
                            if(con_work_mode == access_state.ble_idx)
                            {
                                if(access_state.deep_sleep_flag)
                                {
                                    PRINT("send dis\n");
                                    // 上报蓝牙断开
                                    // 如果当前已经是2.4G模式的话，说明已经发过的断开，不再发。
                                    if( access_state.ble_idx != BLE_INDEX_IDEL)
                                    {
                                        access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
                                    }
                                }
                                else
                                {
                                    //同通道超时断开，不广播，进入睡眠  改为广播5秒后睡眠
                                    // 恢复没有发出去的包；
                                    PRINT("res_num %d\n",BLE_buf_resend_num);
                                    hidEmu_resend_BUF();
                                    hidEmu_adv_enable(ENABLE);
                                    if( hidDevConnSecure )
                                    {
                                        tmos_start_task(hidEmuTaskId, SEND_DISCONNECT_EVT, DISCONNECT_IDEL_SLEEP_EVT_TIMEOUT);
                                    }
//                                    if(tmos_get_task_timer(access_taskId, ACCESS_IDEL_SLEEP_EVT)<IDEL_SLEEP_EVT_TIMEOUT+160)
//                                    {
//                                        access_update_idel_sleep_timeout(IDEL_SLEEP_EVT_TIMEOUT+160);
//                                    }
                                }
                            }
                            else
                            {
                                // 上报蓝牙断开
                                // 如果当前已经是2.4G模式的话，说明已经发过的断开，不再发。
                                if( access_state.ble_idx != BLE_INDEX_IDEL)
                                {
                                    access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
                                }
                                // 进入新蓝牙模式的回连状态
                                access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECTING);
                                hidEmu_adv_enable(ENABLE);
                            }
                        }
                    }
                }
                else if((access_state.ble_idx>BLE_INDEX_IDEL) && (access_state.ble_idx<BLE_INDEX_MAX))
                {
                    con_work_mode = access_state.ble_idx;
                    // 上报蓝牙断开
                    // 如果当前已经是2.4G模式的话，说明已经发过的断开，不再发。
                    if( access_state.ble_idx != BLE_INDEX_IDEL)
                    {
                        access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
                    }
                    // 没绑定过，无法回连
                    access_tran_report(REPORT_CMD_STATE, STATE_RE_CONNECT_FAIL);
                }
                else //还有可能是IDEL模式
                {
                    con_work_mode = access_state.ble_idx;
                    // 上报蓝牙断开
                    // 如果当前已经是2.4G模式的话，说明已经发过的断开，不再发。
                    if( access_state.ble_idx != BLE_INDEX_IDEL)
                    {
                        access_tran_report(REPORT_CMD_STATE, STATE_CON_TERMINATE);
                    }
                }
//                // 记下当前模式  深度睡眠后模式改为idel，所以这里不能同步模式
//                con_work_mode = access_state.ble_idx;
                // 20230831修改 蓝牙模式串口没有上报蓝牙连接断开指令
                hidDevConnSecure = FALSE;
                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                PRINT("Advertising timeout..\n");
            }
            // Enable advertising

            break;

        case GAPROLE_ERROR:
            PRINT("Error %x %x..\n", pEvent->gap.opcode,pEvent->dataUpdate.adType);
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      hidEmu_led_cb_register
 *
 * @brief   hidEmu_led_cb_register
 *
 * @return  none
 */
void hidEmu_receive_cb_register(pfnHidEmuReceiveCB_t cback)
{
    hidEmu_receive_cb = cback;
}

/*********************************************************************
 * @fn      hidEmuRcvReport
 *
 * @brief   Process an incoming HID keyboard report.
 *
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  status
 */
static uint8_t hidEmuRcvReport(uint8_t len, uint8_t *pData)
{
    // verify data length
    if(len == HID_LED_OUT_RPT_LEN)
    {
        // 上报键盘状态灯
        if(hidEmu_receive_cb)
        {
            uint8_t buf[2];
            buf[0] = RF_DATA_LED;
            buf[1] = pData[0];
            hidEmu_receive_cb(buf, 2);
        }
        // set LEDs
        return SUCCESS;
    }
    else
    {
        return ATT_ERR_INVALID_VALUE_SIZE;
    }
}

/*********************************************************************
 * @fn      hidEmuRptCB
 *
 * @brief   HID Dev report callback.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   oper - operation:  read, write, etc.
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  GATT status code.
 */
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData)
{
    uint8_t status = SUCCESS;
    // write
    if(oper == HID_DEV_OPER_WRITE)
    {
        if(uuid == REPORT_UUID)
        {
            // process write to LED output report; ignore others
            if(type == HID_REPORT_TYPE_OUTPUT)
            {
                status = hidEmuRcvReport(*pLen, pData);
            }
        }

        if(status == SUCCESS)
        {
            status = Hid_SetParameter(id, type, uuid, *pLen, pData);
        }
    }
    // read
    else if(oper == HID_DEV_OPER_READ)
    {
        status = Hid_GetParameter(id, type, uuid, pLen, pData);
    }
    // notifications enabled
    else if(oper == HID_DEV_OPER_ENABLE)
    {

    }
    return status;
}

/*********************************************************************
 * @fn      hidEmuEvtCB
 *
 * @brief   HID Dev event callback.
 *
 * @param   evt - event ID.
 *
 * @return  HID response code.
 */
static void hidEmuEvtCB(uint8_t evt)
{
    // process enter/exit suspend or enter/exit boot mode
    if(evt == HID_DEV_SUSPEND_EVT )
    {
#if(DEBUG_3MODE)
        PRINT("HID_SUSPEND\n");
#endif
        PRINT("HID_SUSPEND\n");
        hidDevBattCB(BATT_LEVEL_NOTI_DISABLED);
        access_enter_idel_sleep();
    }
    else if(evt == HID_DEV_EXIT_SUSPEND_EVT )
    {
#if(DEBUG_3MODE)
        PRINT("HID_EXIT_SUSPEND\n");
#endif
        PRINT("HID_EXIT_SUSPEND\n");
        hidDevBattCB(BATT_LEVEL_NOTI_ENABLED);
        peripheral_exit_sleep();
    }
    return;
    return;
}

/*********************************************************************
*********************************************************************/
