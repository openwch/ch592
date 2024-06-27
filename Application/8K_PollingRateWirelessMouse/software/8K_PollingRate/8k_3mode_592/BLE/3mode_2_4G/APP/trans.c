/********************************** (C) COPYRIGHT *******************************
 * File Name          : trans.c
 * Author             : Hikari
 * Version            : V1.0
 * Date               : 2024/01/18
 * Description        :
 *
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include <rf.h>
#include "rf_device.h"
#include "CH59x_common.h"
#include "usb.h"
#include "trans.h"
#include "mouse.h"
#include "peripheral.h"

/*********************************************************************
 * GLOBAL
 */
uint8_t tran_taskID;

#define TRANS_SEND_BUF_NUM      10   //  缓存多少个包
#define TRANS_MAX_BUF_LEN      40   //  单包最长

typedef struct
{
    uint8_t len;
    uint8_t pData[TRANS_MAX_BUF_LEN];
}trans_send_buffer_t;

trans_send_buffer_t trans_send_buffer[TRANS_SEND_BUF_NUM]={0};

uint8_t trans_buf_out_idx=0;
uint8_t trans_buf_data_num=0;

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * @fn      trans_next_buf
 *
 * @brief   trans_next_buf
 *
 * @return  none
 */
void trans_next_buf(void)
{
    trans_buf_data_num--;
    (trans_buf_out_idx==(TRANS_SEND_BUF_NUM-1))?(trans_buf_out_idx=0):(trans_buf_out_idx++);
}

/*********************************************************************
 * @fn      trans_send_data
 *
 * @brief   trans_send_data
 *
 * @return  none
 */
__HIGH_CODE
uint8_t trans_send_data( uint8_t *pData, uint8_t len )
{
    if(work_mode == MODE_USB)
    {
        if(usb_enum_success_flag)
        {
            uint8_t input_idx;
            if( (USB_SleepStatus & HOST_SET_FEATURE) && (USB_SleepStatus & HOST_SET_SUSPEND) )
            {
                if((USB_SleepStatus & HOST_WAKEUP_ENABLE))
                {
                    USB_SleepStatus &= ~HOST_WAKEUP_ENABLE;
                    USB_Wake_up();
                    PRINT("Wake_up\n");
                    USB_READY_FLAG = 1;
                }
            }
            if(trans_buf_data_num==TRANS_SEND_BUF_NUM)
            {
                PRINT("m\n");
                return 0xFE;
            }
            if( len>TRANS_MAX_BUF_LEN)
            {
                PRINT("err 2 l\n");
                return 0xFF;
            }
            input_idx = (trans_buf_out_idx+trans_buf_data_num)%TRANS_SEND_BUF_NUM;
            trans_send_buffer[input_idx].len = len;
            tmos_memcpy((trans_send_buffer[input_idx].pData), pData, len);
            trans_buf_data_num++;
//          PRINT("RF:%d %x %x %x %x\n",len,pData[0],pData[1],pData[2],pData[3] );

            tmos_set_event(tran_taskID, SBP_TRANS_USB_EVT);
        }
    }
    if(work_mode == MODE_2_4G)
    {
        if(RF_check_con_status(RF_CON_CONNECTED))
        {
            // 判断是否空余两个数据包，需要预留一个给其他通道数据使用
            if( !(((RF_DMADESCTypeDef *)pDMATxGet->NextDescAddr)->Status & STA_DMA_ENABLE) )
            {
                if(rf_send_data( pData, len))
                {
//                    PRINT("!\n");
                    return 0xFF;
                }
            }
            else {
//                PRINT("!\n");
                return 0xFE;
            }
        }
        else {
//            PRINT("?\n");
            return 0xFF;
        }
    }

    return 0;

}
/*********************************************************************
 * @fn      trans_process_event
 *
 * @brief   trans 事件处理
 *
 * @param   task_id - 任务ID
 * @param   events  - 事件标志
 *
 * @return  未完成事件
 */
__HIGH_CODE
uint16_t trans_process_event(uint8_t task_id, uint16_t events)
{
    if(events & SBP_TRANS_USB_EVT)
    {
        if(trans_buf_data_num)
        {
            uint8_t state=0;
            switch(trans_send_buffer[trans_buf_out_idx].pData[0])
            {
                case CMD_CLASS_KEYBOARD:
                    state = USB_class_keyboard_report(&trans_send_buffer[trans_buf_out_idx].pData[1],trans_send_buffer[trans_buf_out_idx].len-1);
                    break;

                case CMD_ALL_KEYBOARD:
                    state = USB_all_keyboard_report(&trans_send_buffer[trans_buf_out_idx].pData[1],trans_send_buffer[trans_buf_out_idx].len-1);
                    break;

                case CMD_CONSUMER:
                    state = USB_consumer_report(&trans_send_buffer[trans_buf_out_idx].pData[1],trans_send_buffer[trans_buf_out_idx].len-1);
                    break;

                case CMD_SYS_CTL:
                    state = USB_sys_ctl_report(trans_send_buffer[trans_buf_out_idx].pData[1]);
                    break;

                case CMD_MOUSE:
                    state = USB_mouse_report(&trans_send_buffer[trans_buf_out_idx].pData[1],trans_send_buffer[trans_buf_out_idx].len-1);
                    break;

                case CMD_MANUFACTURER:
                    state = USB_manufacturer_report(&trans_send_buffer[trans_buf_out_idx].pData[1],trans_send_buffer[trans_buf_out_idx].len-1);
                    break;

                case RF_DATA_IAP:
                    state = USB_IAP_report(&trans_send_buffer[trans_buf_out_idx].pData[1],trans_send_buffer[trans_buf_out_idx].len-1);
                    break;

                default:
                    PRINT("code %x\n",trans_send_buffer[trans_buf_out_idx].pData[0]);
                    state = 0xF1;
                    break;
            }
            if( (!state) || (state == 0xF1))
            {
                trans_next_buf();
                tmos_set_event(tran_taskID, SBP_TRANS_USB_EVT);
            }
            else {
                PRINT("!\n");
                tmos_start_task(tran_taskID, SBP_TRANS_USB_EVT, 2);
            }
        }
        return events ^ SBP_TRANS_USB_EVT;
    }
    if(events & SBP_ENTER_SLEEP_DELAY_EVT)
    {
        PRINT("E S\n");
        peripheral_enter_sleep();
        idel_sleep_flag = 1;
        PWR_PeriphWakeUpCfg( DISABLE, RB_SLP_RTC_WAKE, Long_Delay );

        R8_UDEV_CTRL &= ~RB_UD_PORT_EN;                   // 允许USB端口
        PRINT("%x %x\n",GPIOB_ReadPortPin((1<<10)),GPIOB_ReadPortPin((1<<10)));
        GPIOB_ModeCfg((1<<10) ,GPIO_ModeIN_PU);
        GPIOB_ModeCfg((1<<11) ,GPIO_ModeIN_PU);
        if(GPIOB_ReadPortPin((1<<10)))
            GPIOB_ITModeCfg( (1<<10), GPIO_ITMode_LowLevel ); // 下降沿唤醒
        else {
            GPIOB_ITModeCfg( (1<<10), GPIO_ITMode_HighLevel ); //
        }
        if(GPIOB_ReadPortPin((1<<11)))
            GPIOB_ITModeCfg( (1<<11), GPIO_ITMode_LowLevel ); // 下降沿唤醒
        else {
            GPIOB_ITModeCfg( (1<<11), GPIO_ITMode_HighLevel ); //
        }
        PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_USB_WAKE|RB_SLP_GPIO_WAKE, Long_Delay );
        PFIC_EnableIRQ( GPIO_B_IRQn );
//        LowPower_Sleep( RB_PWR_RAM24K | RB_PWR_RAM2K | RB_PWR_EXTEND | RB_XT_PRE_EN ); //只保留30+2K SRAM 供电
//        LowPower_Shutdown(RB_PWR_RAM2K);
//        SYS_ResetExecute();
//        RFIP_WakeUpRegInit();
        R8_UDEV_CTRL |= RB_UD_PORT_EN;                   // 允许USB端口
        PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_RTC_WAKE, Long_Delay );
        PFIC_DisableIRQ( GPIO_B_IRQn );
        if( (USB_SleepStatus & HOST_SET_FEATURE) && (USB_SleepStatus & HOST_SET_SUSPEND) )
        {
            if((USB_SleepStatus & HOST_WAKEUP_ENABLE))
            {
                USB_SleepStatus &= ~HOST_WAKEUP_ENABLE;
                USB_Wake_up();
                PRINT("Wake\n");
                USB_READY_FLAG = 1;
            }
        }
        PRINT("W\n");
        peripheral_exit_sleep();
        return events ^ SBP_ENTER_SLEEP_DELAY_EVT;
    }
    if(events & SBP_ENTER_SLEEP_EVT)
    {
        PRINT("EN\n");
        tmos_start_task(tran_taskID, SBP_ENTER_SLEEP_DELAY_EVT, 1600);
        return events ^ SBP_ENTER_SLEEP_EVT;
    }

    return 0;
}

///*********************************************************************
// * @fn      GPIOB_IRQHandler
// *
// * @brief   GPIOB中断函数,说明被唤醒了
// *
// * @return  none
// */
//__INTERRUPT
//__HIGH_CODE
//void GPIOB_IRQHandler( void )
//{
////    PRINT("Q %x\n",R16_PA_INT_IF);
//    GPIOB_ClearITFlagBit( 0xFFFF );
//    // 停止睡眠
//    R16_PB_INT_EN = 0;
//}

/*********************************************************************
 * @fn      trans_Init
 *
 * @brief   trans_Init
 *
 * @return  none
 */
void trans_Init( void )
{
    tran_taskID = TMOS_ProcessEventRegister(trans_process_event);
//    tmos_start_task(tran_taskID, SBP_TRANS_TEST_EVT, 10*1600);
}

void access_ctl_process( uint8_t ctl_type ){}
void access_pairing_process( uint8_t ctl_type ){}
/******************************** endfile @trans ******************************/
