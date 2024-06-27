/********************************** (C) COPYRIGHT *******************************
* File Name          : nvs_flash.h
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

#ifndef NVS_FLASH_H
#define NVS_FLASH_H

#ifndef SUCCESS
#define SUCCESS	0
#endif

#define LINK_NUM_MAX            2

/*********************************************************************
 * TYPEDEFS
 */

#define USB_VID     0x05AC // 0x1A86 //0x05AC
#define USB_PID     0x024F // 0x8300 //0x024F
#define USB_REV     0x0100

#define  RF_ROLE_ID_INVALD       (0x07)

#define DPI_500                 0
#define DPI_1600                1
#define DPI_5000                2
#define DPI_10000               3
#define DPI_20000               4
#define DPI_MAX                 5


#define LED_MODE_ON             0
#define LED_MODE_MAX            1

#define LED_LIGHT_0             0
#define LED_LIGHT_25            1
#define LED_LIGHT_50            2
#define LED_LIGHT_75            3
#define LED_LIGHT_100           4
#define LED_LIGHT_MAX           5

#define LED_SPEED_1            0
#define LED_SPEED_2            1
#define LED_SPEED_3            2
#define LED_SPEED_4            3
#define LED_SPEED_5            4
#define LED_SPEED_MAX          5

#define SYS_WIN            0
#define SYS_MAC            1

typedef enum
{
    BLE_INDEX_IDEL = 0,
    BLE_INDEX_1,
    BLE_INDEX_2,
    BLE_INDEX_3,
    BLE_INDEX_4,
    BLE_INDEX_5,
    BLE_INDEX_MAX,
}access_ble_idx_t;

//#define MODE_USB            0
//#define MODE_2_4G           1
//#define MODE_BT             2
//
//#define SYS_WIN            0
//#define SYS_MAC            1

#define BLE_BOND_FLAG_1             (1<<0)
#define BLE_BOND_FLAG_2             (1<<1)
#define BLE_BOND_FLAG_3             (1<<2)
#define BLE_BOND_FLAG_4             (1<<3)
#define BLE_BOND_FLAG_5             (1<<4)

typedef struct __PACKED
{
    uint8_t   check_val_A;
    uint8_t   check_val_B;
    uint8_t   led_onoff;
    uint8_t   led_mode;
    uint8_t   dpi;
    uint8_t   ble_idx;
    uint8_t   ble_bond_flag;
    uint8_t   ble_mac_flag;
    uint8_t   ble_name_len;
    uint8_t   ble_name_data[22];
    uint8_t   led_light;
//    uint8_t   sys_mode;   // 由于硬件支持开关控制，无需存储记忆
    uint8_t   work_mode;
    uint8_t   rf_device_id;
    uint8_t   peer_mac[6];
    uint8_t   usb_vid_pid[4];
    uint8_t   usb_prod_info_len;
    uint8_t   usb_prod_info[31];
    uint8_t   capacitance;
    uint8_t   check_sum;
}nvs_flash_info_t;

#define CHECK_VAL_A             0x5A
#define CHECK_VAL_B             0xA5

#define NVS_FLASH_INFO_ADDRESS  0x76000-FLASH_ROM_MAX_SIZE
#define NVS_DFU_FLASH_INFO_ADDRESS  0x75000-FLASH_ROM_MAX_SIZE
/*********************************************************************
 * Global Variables
 */
extern nvs_flash_info_t nvs_flash_info;

#ifndef  bStatus_t
typedef uint8_t                 bStatus_t;
#endif
#ifndef  tmosTaskID
typedef uint8_t                 tmosTaskID;
#endif
#ifndef  tmosEvents
typedef uint16_t                tmosEvents;
#endif
#ifndef  tmosTimer
typedef uint32_t                tmosTimer;
#endif
// Define function type that process event
typedef tmosEvents (*pTaskEventHandlerFn)( tmosTaskID taskID, tmosEvents event );

extern BOOL tmos_memcmp( const void *src1, const void *src2, uint32_t len ); // TRUE - same, FALSE - different
extern void tmos_memcpy( void *dst, const void *src, uint32_t len ); // Generic memory copy.
extern void tmos_memset( void * pDst, uint8_t Value, uint32_t len );
extern bStatus_t tmos_stop_task( tmosTaskID taskID, tmosEvents event );
extern bStatus_t tmos_clear_event( tmosTaskID taskID, tmosEvents event );
extern bStatus_t tmos_start_reload_task( tmosTaskID taskID, tmosEvents event, tmosTimer time );
extern tmosTimer tmos_get_task_timer( tmosTaskID taskID, tmosEvents event );
extern BOOL tmos_start_task( tmosTaskID taskID, tmosEvents event, tmosTimer time );
extern bStatus_t tmos_set_event( tmosTaskID taskID, tmosEvents event );
extern tmosTaskID TMOS_ProcessEventRegister( pTaskEventHandlerFn eventCb );


/*********************************************************************
 * FUNCTIONS
 */
void nvs_flash_init(void);

void nvs_flash_store(void);

/*********************************************************************
*********************************************************************/

#endif



