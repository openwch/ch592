/********************************** (C) COPYRIGHT ******************************
* File Name         : wchrf.h
* Author            : WCH
* Version           : V1.10
* Date              : 2023/11/27
* Description       : head file
*******************************************************************************/



/******************************************************************************/
#ifndef __WCH_RF_H
#define __WCH_RF_H

#include "stdint.h"
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOOL
typedef unsigned char           BOOL;
#endif
#ifndef TRUE
  #define TRUE                  1
#endif
#ifndef FALSE
  #define FALSE                 0
#endif
#ifndef NULL
  #define NULL                  0
#endif
#ifndef SUCCESS
#define  SUCCESS                0x00
#endif
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

typedef uint32_t rfRole_States_t;
// Define function type that generate a random seed callback
typedef uint32_t (*pfnSrandCB)( void );
// Define function type that switch to idle mode callback
typedef uint32_t (*pfnIdleCB)( uint32_t );
// Define function type that run LSI clock calibration callback
typedef void (*pfnLSICalibrationCB)( void );
// Define function type that library status callback.
typedef void (*pfnLibStatusErrorCB)( uint8_t code, uint32_t status );
// Define function type that process event
typedef tmosEvents (*pTaskEventHandlerFn)( tmosTaskID taskID, tmosEvents event );
// Define function type that read flash
typedef uint32_t (*pfnFlashReadCB)( uint32_t addr, uint32_t num, uint32_t *pBuf );
// Define function type that write flash
typedef uint32_t (*pfnFlashWriteCB)( uint32_t addr, uint32_t num, uint32_t *pBuf );
// Define function type that get system clock count
typedef uint32_t (*pfnGetSysClock)( void );
// Define function type that enable/disable system clock interrupt
typedef void (*pfnSetSysClockIRQ)( void );
// Define function type that change the timer trigger value
typedef void (*pfnSetSysClockTign)( int32_t val );
// Define function type that rfRole process callback
typedef void (*pfnRfRoleProcess)( rfRole_States_t status, uint8_t id );

/* tmos config struct */
typedef struct tag_ble_config
{
    uint32_t MEMAddr;               // library memory start address
    uint16_t MEMLen;                // library memory size
    uint8_t TaskMaxCount;
    pfnSetSysClockIRQ enableTmosIrq;
    pfnSetSysClockIRQ disableTmosIrq;
    pfnSrandCB srandCB;             // Register a program that generate a random seed
    pfnIdleCB idleCB;               // Register a program that set idle
    pfnLibStatusErrorCB staCB;      // Register a program that library status callback
 } tmosConfig_t; // Library initialization call BLE_LibInit function


/* TMOS clock config struct */
typedef struct tag_ble_clock_config
{
    pfnGetSysClock getClockValue;  // TMOS系统时间
    uint32_t ClockMaxCount;         // The maximum count value
    uint16_t ClockFrequency;        // The timing clock frequency(Hz)
    uint16_t ClockAccuracy;         // The timing clock accuracy(ppm)

    uint32_t Clock1Frequency;   // 时钟频率 kHz
    pfnGetSysClock getClock1Value; // RF通信管理时间 （精度要求更高）
    pfnSetSysClockIRQ SetPendingIRQ;  // RF通信管理相关中断
    pfnSetSysClockTign SetTign;  // RF通信管理 定时器触发值校准
}tmosTimeConfig_t;

/* pa control config struct */
typedef struct tag_ble_pa_control_config
{
    uint32_t txEnableGPIO;        // tx enable gpio register
    uint32_t txDisableGPIO;       // tx disable gpio register
    uint32_t tx_pin;              // tx pin define
    uint32_t rxEnableGPIO;        // rx enable gpio register
    uint32_t rxDisableGPIO;       // rx disable gpio register
    uint32_t rx_pin;              // tx pin define
} blePaControlConfig_t;

/* RF DMA structure definition */
typedef struct
{
    uint32_t Status;         /* Status */
#define   STA_DMA_ENABLE         0x80000000
#define   STA_LEN_MASK           0x00000FFF

    uint32_t BufferSize;     /* Buffer lengths */
    uint32_t BufferAddr;     /* address pointer */
    uint32_t NextDescAddr;   /* next descriptor address pointer */
} RF_DMADESCTypeDef;

typedef struct
{
    int8_t TxPower;
    RF_DMADESCTypeDef *pTx;
    RF_DMADESCTypeDef *pRx;
    pfnRfRoleProcess rfProcessCB;
    uint32_t processMask;
#define    RF_STATE_RX             (1<<0)
#define    RF_STATE_RBU            (1<<1)
#define    RF_STATE_TX_FINISH      (1<<4)
#define    RF_STATE_TX_IDLE        (1<<5)
#define    RF_STATE_RX_RETRY       (1<<6)

} rfRoleConfig_t;

// defined for all task
#define SYS_EVENT_MSG                   (0x8000)  // A message is waiting event
#define INVALID_TASK_ID                 0xFF      // Task ID isn't setup properly
#define TASK_NO_TASK                    0xFF

typedef struct
{
    uint8_t event;
    uint8_t status;
} tmos_event_hdr_t;

/*********************************************************************
 * GLOBAL MACROS
 */
#define  VER_FILE            "CH59x_RF_FAST_LIB_V1.1"

extern const uint8_t VER_LIB[];  // LIB version
#define SYSTEM_TIME_MICROSEN            625   // unit of process event timer is 625us
#define MS1_TO_SYSTEM_TIME(x)  ((x)*1000/SYSTEM_TIME_MICROSEN)   // transform unit in ms to unit in 625us ( attentional bias )
#define TMOS_TIME_VALID                (30*1000*1000)  // the maximum task time = RTC MAX clock - TMOS_TIME_VALID

/* takes a byte out of a uint32_t : var - uint32_t,  ByteNum - byte to take out (0 - 3) */
#define BREAK_UINT32( var, ByteNum ) (uint8_t)((uint32_t)(((var) >>((ByteNum) * 8)) & 0x00FF))
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define HI_UINT8(a)  (((a) >> 4) & 0x0F)
#define LO_UINT8(a)  ((a) & 0x0F)
#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((uint32_t)(((uint32_t)(Byte0) & 0x00FF) \
          + (((uint32_t)(Byte1) & 0x00FF) << 8) \
          + (((uint32_t)(Byte2) & 0x00FF) << 16) \
          + (((uint32_t)(Byte3) & 0x00FF) << 24)))
#define BUILD_UINT16(loByte, hiByte) ((uint16_t)(((loByte) & 0x00FF)|(((hiByte) & 0x00FF)<<8)))

#define ACTIVE_LOW        !
#define ACTIVE_HIGH       !!            // double negation forces result to be '1'

#ifndef BV
#define BV(n)      (1 << (n))
#endif

#ifndef BF
#define BF(x,b,s)  (((x) & (b)) >> (s))
#endif

#ifndef MIN
#define MIN(n,m)   (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
#define MAX(n,m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef ABS
#define ABS(n)     (((n) < 0) ? -(n) : (n))
#endif

/* TxPower define(Accuracy:±2dBm) */
#define LL_TX_POWEER_MINUS_20_DBM       0x01
#define LL_TX_POWEER_MINUS_15_DBM       0x03
#define LL_TX_POWEER_MINUS_10_DBM       0x05
#define LL_TX_POWEER_MINUS_8_DBM        0x07
#define LL_TX_POWEER_MINUS_5_DBM        0x0B
#define LL_TX_POWEER_MINUS_3_DBM        0x0F
#define LL_TX_POWEER_MINUS_1_DBM        0x13
#define LL_TX_POWEER_0_DBM              0x15
#define LL_TX_POWEER_1_DBM              0x1B
#define LL_TX_POWEER_2_DBM              0x23
#define LL_TX_POWEER_3_DBM              0x2B
#define LL_TX_POWEER_4_DBM              0x3B



#define  RF_MAX_DATA_LEN              251

// defined for properties
// whitening off enable
#define  BB_WHITENING_OFF             (1<<0)
// PHY_MODE_TYPE  @PHY AUX TYPE(2b)
#define  PHY_MODE_MODE_MASK       (0x30)
#define  PHY_MODE_PHY_1M              (0<<4)
#define  PHY_MODE_PHY_2M              (1<<4)
#define  PHY_MODE_PHY_CODED_S8        (2<<4)
#define  PHY_MODE_PHY_CODED_S2        (3<<4)

#define FAILURE                         0x01   //!< Failure
#define INVALIDPARAMETER                0x02   //!< Invalid request field
#define INVALID_TASK                    0x03   //!< Task ID isn't setup properly
#define MSG_BUFFER_NOT_AVAIL            0x04   //!< No buffer is available.
#define INVALID_MSG_POINTER             0x05   //!< No message pointer.
#define INVALID_EVENT_ID                0x06   //!< Invalid event id.
#define INVALID_TIMEOUT                 0x07   //!< Invalid timeout.
#define NO_TIMER_AVAIL                  0x08   //!< No event is available.
#define NV_OPER_FAILED                  0x0A   //!< read a data item to NV failed.
#define INVALID_MEM_SIZE                0x0B   //!< The tokens take up too much space and don't fit into Advertisement data and Scan Response Data

/** BLE_STATUS_VALUES BLE Default BLE Status Values
 * returned as bStatus_t
 */
#define bleInvalidTaskID                INVALID_TASK  //!< Task ID isn't setup properly
#define bleEecKeyRequestRejected        0x06   //!< key missing
#define bleNotReady                     0x10   //!< Not ready to perform task
#define bleAlreadyInRequestedMode       0x11   //!< Already performing that task
#define bleIncorrectMode                0x12   //!< Not setup properly to perform that task
#define bleMemAllocError                0x13   //!< Memory allocation error occurred
#define bleNotConnected                 0x14   //!< Can't perform function when not in a connection
#define bleNoResources                  0x15   //!< There are no resource available
#define blePending                      0x16   //!< Waiting
#define bleTimeout                      0x17   //!< Timed out performing function
#define bleInvalidRange                 0x18   //!< A parameter is out of range
#define bleLinkEncrypted                0x19   //!< The link is already encrypted
#define bleProcedureComplete            0x1A   //!< The Procedure is completed
#define bleInvalidMtuSize               0x1B   //!< SDU size is larger than peer MTU.

/* package type */
#define  PKT_HEAD_LEN          4
#define  PKT_ACK_LEN           (PKT_HEAD_LEN-2)
#define  PKT_RESV_LEN           PKT_ACK_LEN

/* device id type */
#define  RF_ROLE_ID_INVALD       (0x07)
#define  RF_ROLE_ID_MASK         (0x07)
#define  RF_ROLE_BOUND_MAX       (0x07)
#define  RF_ROLE_BOUND_ID        RF_ROLE_ID_INVALD

/* package type */
typedef struct __attribute__((packed))
{
    uint8_t type;                 //!< bit0-2:device id
                                  //!< bit3-7:reserved
    uint8_t length;               //!< data length
    uint8_t seq;                  //!< reserved
    uint8_t resv;                 //!< reserved
} rfPackage_t;

/* rfRole parameter */
typedef struct
{
    uint32_t accessAddress;       //!< access address,32bit PHY address
    uint32_t crcInit;             //!< crc initial value
    uint32_t frequency;           //!< rf frequency (2400000kHz-2483500kHz)
    uint32_t properties;          //!< bit0: 0-whitening on 1-whitening off
                                  //!< BIT4-5 00-1M  01-2M
    uint32_t rxMaxLen;            //!< The maximum length of data received
} rfRoleParam_t;

typedef struct
{
    bStatus_t status; //!< Status for the connection
    /* SUCESS
     * bleTimeout:When the connection times out, it automatically switches to the bindable state
     * FAILURE:If the device binding fails on the device side, the application layer needs to restart the binding */
    uint8_t role;     //!< the role of the binding
    uint8_t devId;    //!< The ID number of the binding
    uint8_t devType;  //!< The device type of the binding
    uint8_t periTime; //!< reserved
    uint8_t hop;      //!< Frequency hopping mode
    uint8_t PeerInfo[6]; //!< Information about the peer device
} staBound_t;

// Define function type that rfRole bound  process callback
typedef void (*pfnRfRoleBoundCB)( staBound_t * );

/* Definition of roles */
#define  RF_ROLE_RX_MOD0      0 //!< host


#define  RF_ROLE_TX_MOD0      1 //!< device
#define  RF_ROLE_TX_MOD1      3 //!< device

/* Definition of frequency hopping mode type */
#define  RF_HOP_OFF           0  //!< Fixed frequency
#define  RF_HOP_BLECS2_MODE   1  //!< Frequency Hopping Mode 1 (same as BLE-CS#2)
#define  RF_HOP_MANUF_MODE    2  //!< Frequency Hopping Mode 2 (Manufacturer-Defined)

/* DEVICE configuration information */
typedef struct
{
    uint8_t devType;  //!< the device type, 0:Not specified
    uint8_t deviceId; //!< the device ID, 7:the ID number is assigned by the host
    uint8_t calVal;  //!< Calibration values
    uint8_t speed;  //!< communication speed(kHz)
    uint16_t timeout;  //!< Connection communication timeout period (in about 1 ms)
    uint8_t OwnInfo[6]; //!< Local Information
    uint8_t PeerInfo[6]; //!< Peer information
    pfnRfRoleBoundCB rfBoundCB;
} rfBoundDevice_t;

/* HOST configuration information */
typedef struct
{
    uint8_t hop; //!< Frequency hopping mode
    int8_t rssi;  //!< reserved
    uint8_t periTime; //!< Fixed value 8
    uint8_t devType;   //!< reserved
    uint16_t timeout; //!< Connection communication timeout period (in about 1 ms)
    uint8_t OwnInfo[6]; //!< Local Information
    uint8_t PeerInfo[6];   //!< Peer information
    pfnRfRoleBoundCB rfBoundCB;
} rfBoundHost_t;

/* listing information */
typedef struct
{
    uint8_t deviceId;  //!< The ID number of the list 0-6:Connection   7:binding
    int8_t rssi;       //!< Minimum RSSI threshold for binding, 0 means unlimited (deviceId=7 valid)
    uint8_t devType;   //!< Specify the bound device type, 0 means unlimited (deviceId=7 valid)
    uint8_t peerInfo[6]; //!< Specify the peer information of the binding, 0 means unlimited (deviceId=7 valid)
} rfRoleList_t;

/* Connection or binding management lists */
typedef struct
{
    uint8_t number;  //!< Number of lists
    rfRoleList_t *pList; //!< listing information
} rfRoleSpeed_t;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 */
uint32_t tmos_rand( void ); // pseudo-random number
BOOL tmos_memcmp( const void *src1, const void *src2, uint32_t len ); // TRUE - same, FALSE - different
BOOL tmos_isbufset( uint8_t *buf, uint8_t val, uint32_t len ); // TRUE if all "val",FALSE otherwise
uint32_t tmos_strlen( char *pString );
void tmos_memset( void * pDst, uint8_t Value, uint32_t len );
void tmos_memcpy( void *dst, const void *src, uint32_t len ); // Generic memory copy.

/**
 * @brief
 *
 * @param   pConf -
 *
 * @return   0 - SUCCESS.
 */
bStatus_t TMOS_Init( tmosConfig_t *pConf );

/**
 * @brief   start a event immediately
 *
 * @param   taskID - task ID of event
 * @param   event - event value
 *
 * @return  0 - SUCCESS.
 */
bStatus_t tmos_set_event( tmosTaskID taskID, tmosEvents event );

/**
 * @brief   clear a event already timeout, cannot be used in it own event function.
 *
 * @param   taskID - task ID of event
 * @param   event - event value
 *
 * @return  0 - SUCCESS.
 */
bStatus_t tmos_clear_event( tmosTaskID taskID, tmosEvents event );

/**
 * @brief   start a event after period of time
 *
 * @param   taskID - task ID to set event for
 * @param   event - event to be notified with
 * @param   time - timeout value
 *
 * @return  TRUE,FALSE.
 */
BOOL tmos_start_task( tmosTaskID taskID, tmosEvents event, tmosTimer time );

/**
 * @brief   This function is called to start a timer to expire in n system clock time.
 *          When the timer expires, the calling task will get the specified event
 *          and the timer will be reloaded with the timeout value.
 *
 * @param   taskID - task ID to set timer for
 * @param   event - event to be notified with
 * @param   time - timeout value
 *
 * @return  SUCCESS, or NO_TIMER_AVAIL.
 */
bStatus_t tmos_start_reload_task( tmosTaskID taskID, tmosEvents event, tmosTimer time );

/**
 * @brief   stop a event
 *
 * @param   taskID - task ID of event
 * @param   event - event value
 *
 * @param   None.
 *
 * @return  SUCCESS.
 */
bStatus_t tmos_stop_task( tmosTaskID taskID, tmosEvents event );

/**
 * @brief   get last period of time for this event
 *
 * @param   taskID - task ID of event
 * @param   event - event value
 *
 * @return  the timer's tick count if found, zero otherwise.
 */
tmosTimer tmos_get_task_timer( tmosTaskID taskID, tmosEvents event );

/**
 * @brief   send msg to a task,callback events&SYS_EVENT_MSG
 *
 * @param   taskID - task ID of task need to send msg
 * @param  *msg_ptr - point of msg
 *
 * @return  SUCCESS, INVALID_TASK, INVALID_MSG_POINTER
 */
bStatus_t tmos_msg_send( tmosTaskID taskID, uint8_t *msg_ptr );

/**
 * @brief   delete a msg
 *
 * @param  *msg_ptr - point of msg
 *
 * @return  SUCCESS.
 */
bStatus_t tmos_msg_deallocate( uint8_t *msg_ptr );

/**
 * @brief   receive a msg
 *
 * @param   taskID  - task ID of task need to receive msg
 *
 * @return *uint8_t - message information or NULL if no message
 */
uint8_t *tmos_msg_receive( tmosTaskID taskID );

/**
 * @brief   allocate buffer for msg when need to send msg
 *
 * @param   len  - length of msg
 *
 * @return  pointer to allocated buffer or NULL if allocation failed.
 */
uint8_t *tmos_msg_allocate( uint16_t len );

/**
 * @brief   tmos system timer initialization
 *
 * @note    must initialization before call tmos task
 *
 * @param   fnGetClock - system clock select extend input,if NULL select HSE as the clock source
 *
 * @return  SUCCESS if successful, FAILURE if failed.
 */
bStatus_t TMOS_TimerInit( tmosTimeConfig_t *pClockConfig );

/**
 * @brief   interrupt handler.
 *
 * @param   None
 *
 * @return  None
 */
bStatus_t TMOS_TimerIRQHandler( uint32_t *time );

/**
 * @brief   Process system
 *
 * @param   None.
 *
 * @return  None.
 */
void TMOS_SystemProcess( void );

/**
 * @brief   Get current system clock
 *
 * @param   None.
 *
 * @return  current system clock (in 0.625ms)
 */
uint32_t TMOS_GetSystemClock( void );

/**
 * @brief   register process event callback function
 *
 * @param   eventCb-events callback function
 *
 * @return  0xFF - error,others-task id
 */
tmosTaskID TMOS_ProcessEventRegister( pTaskEventHandlerFn eventCb );

/**
 * @brief   read rssi
 *
 * @param   None.
 *
 * @return  the value of rssi.
 */
void LLE_LibIRQHandler( void );

/**
 * @brief   read rssi
 *
 * @param   None.
 *
 * @return  the value of rssi.
 */
void BB_LibIRQHandler( void );

/**
 * @brief   read rssi
 *
 * @param   None.
 *
 * @return  the value of rssi.
 */
int8_t RFIP_ReadRssi( void );

/**
 * @brief   read crc state
 *
 * @param   None.
 *
 * @return  the value of crc state.
 */
uint8_t RFIP_ReadCrc( void );

/**
 * @brief   set output power level@ TxPower
 *
 * @param   None.
 *
 * @return  the value of crc state.
 */
void RFIP_SetTxPower( uint8_t val );

/**
 * @brief   pa control init
 *          
 * @note    Can't be called until  role Init
 *
 * @param   paControl - pa control parameters(global variable)
 *
 * @return  Command Status.
 */
void RFIP_PAControlInit( blePaControlConfig_t *paControl );

/**
 * @brief   rf calibration
 *
 * @param   None 
 *
 * @return  None
 */
void RFIP_Calibration( void );

/**
 * @brief   reinitialize the rfip register after sleep wake-up
 *
 * @param   None
 *
 * @return  None
 */
void RFIP_WakeUpRegInit( void );

/**
 * @brief   used to set rf TxCtune value
 *
 * @param   pParm(in) - Must provide length of parameter followed by 6 bytes parameter
 *
 * @return  Command Status.
 */
bStatus_t RFEND_TXCtuneSet( uint8_t *pParm );

/**
 * @brief   used to get rf TxCtune value
 *
 * @param   pParm(out) - length of parameter(6) followed by 6 bytes parameter
 *
 * @return  Command Status.
 */
bStatus_t RFEND_TXCtuneGet( uint8_t *pParm );

/**
 * @brief   used to output single channel
 *
 * @param   pParm(in) - set channel
 *
 * @return  Command Status.
 */
void RFIP_SingleChannel( uint32_t ch );

/**
 * @brief   used to turn off single channel
 *
 * @param   None
 *
 * @return  None.
 */
void RFIP_TestEnd( void );

/**
 * @brief   library initial
 *
 * @param   clock - system clock
 *
 * @return  0-success.
 */
bStatus_t RFRole_LibInit( rfRoleConfig_t *pConf );

/**
 * @brief   used to power off rf
 *
 * @param   None
 *
 * @return  0-success.
 */
bStatus_t RFRole_Shut(void );

/**
 * @brief   used to set rf-role parameter
 *
 * @param   pConf
 *
 * @return  0-success.
 */
bStatus_t RFRole_SetParam( rfRoleParam_t *pConf );

/**
 * @brief   set Connection or binding management lists
 *
 * @param   pList_t - management lists(global variable)
 *
 * @return  0-success.
 */
bStatus_t RFBound_SetSpeedType( rfRoleSpeed_t *pList_t );

/**
 * @brief   start host mode
 *
 * @param   None.
 *
 * @return  0-success.
 */
bStatus_t RFBound_StartHost( rfBoundHost_t *pConfig );

/**
 * @brief   start device mode
 *
 * @param   None.
 *
 * @return  0-success.
 */
bStatus_t RFBound_StartDevice( rfBoundDevice_t *pConfig );

/**
 * @brief   clear data list
 *
 * @param   dev_id
 *
 * @return  0-success.
 */
bStatus_t RFRole_ClearTxData( uint8_t dev_id );

/**
 * @brief   clear data list
 *
 * @param   dev_id
 *
 * @return  0-success.
 */
bStatus_t RFRole_ClearRxData( uint8_t dev_id );

/*
 * END @ API
 */
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
