/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : RF_PHY例程：2.4G接收，收到数据后自动应答并打印
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "RF_PHY.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define RF_AUTO_MODE_EXAM       1

uint8_t taskID;
uint8_t TX_DATA[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};

volatile uint8_t tx_end_flag=0;
volatile uint8_t rx_end_flag=0;

/* RF_2G4StatusCallBack runs in ISR context, so it must not call the blocking
 * PRINT (UART) nor busy-wait. Received frames are buffered here and printed
 * from RF_ProcessEvent (task context) instead. */
#define SBP_RF_PRINT_EVT      (1 << 5)
#define RF_PRINT_NONE         0
#define RF_PRINT_RX_RX        2
#define RF_PRINT_BUF_SIZE     251

static volatile uint8_t rf_print_kind = RF_PRINT_NONE;
static volatile uint8_t rf_print_crc = 0;
static volatile int8_t  rf_print_rssi = 0;
static volatile uint8_t rf_print_len = 0;
static volatile uint8_t rf_print_buf[RF_PRINT_BUF_SIZE];

/*********************************************************************
 * @fn      RF_Wait_Rx_End
 *
 * @brief   自动模式等待应答发送完成，必须在RAM中等待，等待时可以执行用户代码，但需要注意执行的代码必须运行在RAM中，否则影响发送
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void RF_Wait_Rx_End()
{
    uint32_t i=0;
    while(!rx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // 约5ms超时
        if(i>(FREQ_SYS/1000))
        {
            rx_end_flag = TRUE;
        }
    }
}

/*********************************************************************
 * @fn      RF_2G4StatusCallBack
 *
 * @brief   RF 状态回调，此函数在中断中调用。注意：不可在此函数中直接调用RF接收或者发送API，需要使用事件的方式调用
 *          在此回调中直接使用或调用函数涉及到的变量需注意，此函数在中断中调用。
 *
 * @param   sta     - 状态类型
 * @param   crc     - crc校验结果
 * @param   rxBuf   - 数据buf指针
 *
 * @return  none
 */
void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
    {
        case RX_MODE_RX_DATA:
        {
            if (crc == 0) {
                uint8_t i;

                /* ISR context: no busy-wait / blocking print here.
                 * Buffer the frame and print it from the task. */
                rf_print_len = rxBuf[1];
                if (rf_print_len > RF_PRINT_BUF_SIZE) {
                    rf_print_len = RF_PRINT_BUF_SIZE;
                }
                for (i = 0; i < rf_print_len; i++) {
                    rf_print_buf[i] = rxBuf[i + 2];
                }
                rf_print_rssi = (int8_t)rxBuf[0];
                
                }
            rf_print_kind = RF_PRINT_RX_RX;
            rf_print_crc = crc;
            tmos_set_event(taskID, SBP_RF_PRINT_EVT);
#if(!RF_AUTO_MODE_EXAM)
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
#endif
            break;
        }
        case RX_MODE_TX_FINISH:
        {
#if(RF_AUTO_MODE_EXAM)
            rx_end_flag = TRUE;
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
#endif
            break;
        }
        case RX_MODE_TX_FAIL:
        {
#if(RF_AUTO_MODE_EXAM)
            rx_end_flag = TRUE;
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
#endif
            break;
        }
    }
}

/*********************************************************************
 * @fn      RF_ProcessEvent
 *
 * @brief   RF 事件处理
 *
 * @param   task_id - 任务ID
 * @param   events  - 事件标志
 *
 * @return  未完成事件
 */
uint16_t RF_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if(events & SBP_RF_RF_RX_EVT)
    {
        uint8_t state;
        RF_Shut();
        TX_DATA[0]++;
#if(RF_AUTO_MODE_EXAM)
        rx_end_flag = FALSE;
#endif
        state = RF_Rx(TX_DATA, 10, 0xFF, 0xFF);
        PRINT("RX mode.state = %x\n", state);
        return events ^ SBP_RF_RF_RX_EVT;
    }
    /* Print the deferred RX result here (task context), never inside the ISR. */
    if(events & SBP_RF_PRINT_EVT)
    {
        uint8_t i;

        if(rf_print_kind == RF_PRINT_RX_RX)
        {
            if(rf_print_crc == 0)
            {
                PRINT("rx recv, rssi: %d\n", rf_print_rssi);
                PRINT("len:%d-", rf_print_len);
                for(i = 0; i < rf_print_len; i++)
                {
                    PRINT("%x ", rf_print_buf[i]);
                }
                PRINT("\n");
            }
            else
            {
                if(rf_print_crc & (1<<0))
                {
                    PRINT("crc error\n");
                }
                if(rf_print_crc & (1<<1))
                {
                    PRINT("match type error\n");
                }
            }
        }
        rf_print_kind = RF_PRINT_NONE;
        return events ^ SBP_RF_PRINT_EVT;
    }
    return 0;
}

/*********************************************************************
 * @fn      RF_Init
 *
 * @brief   RF 初始化
 *
 * @return  none
 */
void RF_Init(void)
{
    uint8_t    state;
    rfConfig_t rf_Config;

    tmos_memset(&rf_Config, 0, sizeof(rfConfig_t));
    taskID = TMOS_ProcessEventRegister(RF_ProcessEvent);
    rf_Config.accessAddress = 0x71764129; // 禁止使用0x55555555以及0xAAAAAAAA ( 建议不超过24次位反转，且不超过连续的6个0或1 )
    rf_Config.CRCInit = 0x555555;
    rf_Config.Channel = 39;
    rf_Config.Frequency = 2480000;
#if(RF_AUTO_MODE_EXAM)
    rf_Config.LLEMode = LLE_MODE_AUTO;
#else
    rf_Config.LLEMode = LLE_MODE_BASIC | LLE_MODE_EX_CHANNEL; // 使能 LLE_MODE_EX_CHANNEL 表示 选择 rf_Config.Frequency 作为通信频点
#endif
    rf_Config.rfStatusCB = RF_2G4StatusCallBack;
    rf_Config.RxMaxlen = 251;
    state = RF_Config(&rf_Config);
    PRINT("rf 2.4g init: %x\n", state);
    /* 打印演示配置信息 */
    PRINT("\n==============================================\n");
    PRINT("  2.4G RF PHY RX\n");
#if( RF_AUTO_MODE_EXAM )
    PRINT("  mode     : AUTO\n");
#else
    PRINT("  mode     : BASIC\n");
#endif
    PRINT("  freq     : %d.%03d MHz\n", rf_Config.Frequency / 1000, rf_Config.Frequency % 1000);
    PRINT("  channel  : %d\n", rf_Config.Channel);
    PRINT("  addr     : %08x\n", rf_Config.accessAddress);
    PRINT("  rxMaxlen : %d\n", rf_Config.RxMaxlen);
    PRINT("==============================================\n");
    { // RX mode
#if(RF_AUTO_MODE_EXAM)
        rx_end_flag = FALSE;
#endif
        state = RF_Rx(TX_DATA, 10, 0xFF, 0xFF);
        PRINT("RX mode.state = %x\n", state);
    }
}

/******************************** endfile @ main ******************************/
