/********************************** (C) COPYRIGHT *******************************
 * File Name          : mouse.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        :
 *
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "peripheral.h"
#include "mouse.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define SAMPLING_RATE           12000

#define USB_2_4G_REPORTING_RATE      8000
#define BLE_REPORTING_RATE      133

#define USB_2_4G_MOTION_DIV      (SAMPLING_RATE/USB_2_4G_REPORTING_RATE)
#define BLE_MOTION_DIV      (SAMPLING_RATE/BLE_REPORTING_RATE)

#define KEY_SCAN_DIV    6
#define WHEEL_DIV       12

// 3分钟一档睡眠，30分钟二档睡眠,未连接状态，30s进入深度睡眠
#define IDEL_SLEEP_TIMEOUT              1600*10
#define DEEP_SLEEP_TIMEOUT              1600*60*3
#define IDEL_DEEP_SLEEP_TIMEOUT         1600*30

// 灯闪烁30s，频率为2Hz，配对30s超时，
#define PAIRING_LED_BLINK_INTERVAL            1600*1/4
#define PAIRING_LED_BLINK_TIMEOUT             1600*30

// 如果BLE或者2.4G断开连接，闪烁30s，频率1Hz,则30s后关闭所有背光，进入低功耗Sleep休眠
#define DISCONNECT_LED_BLINK_INTERVAL            1600*1/2
#define DISCONNECT_LED_BLINK_TIMEOUT             1600*30

// RGB指示常量后3秒关闭，省电
#define RGB_LED_OFF_TIMEOUT             1600*3

volatile uint8_t mouse_scan_flag=0;

uint8_t work_mode = MODE_USB;

#define MOUSE_DATA_LEN       5   // 1B命令码+1B按键+2B移动+1B滚轮
uint8_t mouse_data[MOUSE_DATA_LEN]={CMD_MOUSE,0,0,0,0};

#define KEY_SCAN_FILTER_COUNT_MAX       1   // 0表示但单次采样即输出，n表示n+1次采样相等才输出键值

#define KEY_DETECTE_DELAY_US        5
#define KEY_DETECTE_DELAY()      DelayUs(KEY_DETECTE_DELAY_US)

#define KEY_TABLE_R        4
#define KEY_TABLE_C        2

#define TABLE_IDX_SW_BLE        (3)
#define TABLE_IDX_SW_24G        (2)

// 长按按键个数
#define KEY_TIMEOUT_NUM        1

const uint8_t KEY_TABLE[KEY_TABLE_R*KEY_TABLE_C]=
    {
        /* R0    R1       */

  /* C0   DPI    MB   2.4G  BLE     */
          0xF0, 0x04, 0xF1, 0xF2,
  /* C1    LB    RB   Back Forward      */
          0x01, 0x02, 0xE4, 0xE3
    };

// 需要长按计时的按键
const uint8_t KEY_TIMER_TABLE[KEY_TABLE_R*KEY_TABLE_C]=
    {
        /* R0    R1       */

  /* C0   DPI    MB   2.4G  BLE     */
          0x01, 0x00, 0x00, 0x00,
  /* C1    LB    RB Forward Back       */
          0x00, 0x00, 0x00, 0x00
    };

const uint32_t KEY_C_INDEX[KEY_TABLE_C]=
    {
        KEY_C_0, KEY_C_1
    };
const uint32_t KEY_R_INDEX[KEY_TABLE_R]=
    {
        KEY_R_0, KEY_R_1, KEY_R_2, KEY_R_3
    };

//长按计时列表
uint32_t key_timer_list[KEY_TIMEOUT_NUM]={0};
//长按超时对应按键
const uint8_t KEY_TIMEOUT_TABLE[KEY_TIMEOUT_NUM]=
    {
  /* C   DPI  */
        0xF0
    };
//长按超时列表(s)
const uint8_t KEY_TIMEOUT_LIST[KEY_TIMEOUT_NUM]=
    {
  /* C  DPI*/
         0
    };
uint8_t key_scan_filter_count=0;
uint8_t key_data_scan_new[KEY_TABLE_R*KEY_TABLE_C];
uint8_t key_data_scan_filter[KEY_TABLE_R*KEY_TABLE_C];
uint8_t key_data_scan_old[KEY_TABLE_R*KEY_TABLE_C]={0};
uint8_t key_data_scan_byte[KEY_TABLE_R*KEY_TABLE_C];

uint8_t key_scan_filter_count_max=KEY_SCAN_FILTER_COUNT_MAX;

uint32_t key_press_time=0;

volatile uint8_t idel_sleep_flag=0; // 一档睡眠
volatile uint8_t deep_sleep_flag=0; // 二档睡眠
volatile uint8_t enter_sleep_flag=0; //

uint8_t mouse_taskID=0;

uint8_t vbat_info=0;

uint8_t led_blink_range=LED_BLINK_ALL;

signed short RoughCalib_Value = 0; // ADC粗调偏差值

connect_status_t connect_state=CON_IDEL;

const uint16_t DPI_VALUE[DPI_MAX]=
    {
        500/50-1, 1800/50-1, 5000/50-1, 10000/50-1, 20000/50-1
    };

#define RGB_R_IDX              (0)
#define RGB_G_IDX              (1)
#define RGB_B_IDX              (2)
#define RGB_MAX_IDX              (3)

const uint8_t DPI_VALUE_RGB[DPI_MAX][RGB_MAX_IDX]=
    {
        {128,255,255},
        {0,255,0},
        {64,128,255},
        {0,0,255},
        {255,0,255}
    };

uint8_t mouse_get_batt_info(void);
void mouse_motion_scan(void);
/*********************************************************************
 * @fn      mouse_process_event
 *
 * @brief   mouse_process_event 事件处理
 *
 * @param   task_id - 任务ID
 * @param   events  - 事件标志
 *
 * @return  未完成事件
 */
uint16_t mouse_process_event(uint8_t task_id, uint16_t events)
{

    if(events & IDEL_SLEEP_EVENT)
    {
        PRINT("@S\n");
        idel_sleep_flag = TRUE;
        access_enter_idel_sleep();
        return events ^ IDEL_SLEEP_EVENT;
    }

    if(events & DEEP_SLEEP_EVENT)
    {
        PRINT("@Deep_sleep\n");
        deep_sleep_flag = TRUE;
        access_enter_deep_sleep();
        return events ^ DEEP_SLEEP_EVENT;
    }

    if(events & MOUSE_LED_BLINK_EVENT)
    {
        PRINT("d %d\n",nvs_flash_info.led_light);
        if(R8_PWM_OUT_EN & led_blink_range)
        {
            PWMX_ACTOUT(led_blink_range, 0, Low_Level, DISABLE);
        }
        else
        {
            PWMX_ACTOUT(VDR_PWM_R, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_R_IDX], Low_Level, ENABLE);
            PWMX_ACTOUT(VDR_PWM_G, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_G_IDX], Low_Level, ENABLE);
            PWMX_ACTOUT(VDR_PWM_B, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_B_IDX], Low_Level, ENABLE);
        }
        return events ^ MOUSE_LED_BLINK_EVENT;
    }

    if(events & MOUSE_LED_TIMEOUT_EVENT)
    {
        tmos_stop_task(mouse_taskID, MOUSE_LED_BLINK_EVENT);
        if(nvs_flash_info.led_onoff)
        {
            PRINT("R %d\n",DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_R_IDX]);
            PRINT("G %d\n",DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_G_IDX]);
            PRINT("B %d\n",DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_B_IDX]);

            PWMX_ACTOUT(VDR_PWM_G, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_G_IDX], Low_Level, ENABLE);
            PWMX_ACTOUT(VDR_PWM_B, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_B_IDX], Low_Level, ENABLE);
            PWMX_ACTOUT(VDR_PWM_R, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_R_IDX], Low_Level, ENABLE);
            tmos_start_task(mouse_taskID, MOUSE_LED_OFF_EVENT, RGB_LED_OFF_TIMEOUT);
            switch(nvs_flash_info.led_mode)
            {
                case LED_MODE_ON:
                {
                    break;
                }
            }
        }
        else
        {
            PWMX_ACTOUT(LED_BLINK_ALL, 0, Low_Level, DISABLE);
        }
        return events ^ MOUSE_LED_TIMEOUT_EVENT;
    }

    if(events & MOUSE_LED_OFF_EVENT)
    {
        PWMX_ACTOUT(LED_BLINK_ALL, 0, Low_Level, DISABLE);
        return events ^ MOUSE_LED_OFF_EVENT;
    }

    if(events & MOUSE_VBAT_INFO_EVENT)
    {
        vbat_info = mouse_get_batt_info();
//        PRINT("bat： %d%%\n",vbat_info);
        return events ^ MOUSE_VBAT_INFO_EVENT;
    }

    return 0;
}

/*******************************************************************************
 * @fn      peripheral_sleep_update
 *
 * @brief   peripheral_sleep_update
 *
 * @return  None.
 */
__HIGH_CODE
void peripheral_sleep_update()
{
    if( work_mode != MODE_USB && work_mode != MODE_2_4G)
    {
        if(idel_sleep_flag||deep_sleep_flag)
        {
            peripheral_exit_sleep();
        }
        if( connect_state == CON_CONNECTED)
        {
            tmos_start_task(mouse_taskID, IDEL_SLEEP_EVENT, IDEL_SLEEP_TIMEOUT);
            tmos_start_task(mouse_taskID, DEEP_SLEEP_EVENT, DEEP_SLEEP_TIMEOUT);
        }
        else
        {
            tmos_start_task(mouse_taskID, DEEP_SLEEP_EVENT, IDEL_DEEP_SLEEP_TIMEOUT);
        }
    }
    else
    {
        tmos_stop_task(mouse_taskID, IDEL_SLEEP_EVENT);
        tmos_stop_task(mouse_taskID, DEEP_SLEEP_EVENT);
    }
}

/*******************************************************************************
 * @fn      peripheral_enter_sleep
 *
 * @brief   peripheral_enter_sleep
 *
 * @return  None.
 */
void peripheral_enter_sleep()
{
    uint32_t pin_state;
    uint32_t need_reset_bit=0;
    PWMX_LED_ALL_OFF();
    VBAT_CHECK_DISABLE();
    for(uint8_t i=0; i<KEY_TABLE_C; i++)
    {
        GPIOA_SetBits(KEY_C_0|KEY_C_1);
        GPIOA_ResetBits( KEY_C_INDEX[i]);
        KEY_DETECTE_DELAY();
        if(GPIOB_ReadPortPin(KEY_R_0|KEY_R_1|KEY_R_2|KEY_R_3) ==
            (KEY_R_0|KEY_R_1|KEY_R_2|KEY_R_3))
        {
            need_reset_bit |= KEY_C_INDEX[i];
        }
    }
//    PRINT("> %x %x %x\n",need_reset_bit,KEY_R_0|KEY_R_1|KEY_R_2|KEY_R_3|KEY_R_4|KEY_R_5
//        |KEY_R_6|KEY_R_7|KEY_R_8|KEY_R_9|KEY_R_10|KEY_R_11|KEY_R_12
//        |KEY_R_13|KEY_R_14|KEY_R_15|KEY_R_16|KEY_R_17,GPIOA_ReadPortPin(KEY_C_0|KEY_C_1|KEY_C_2|KEY_C_3|KEY_C_4|KEY_C_5));
    // 由于切换模式开关挂载在按键扫描下，所以KEY_R_14相关的其余四个按键不支持唤醒
    if(!need_reset_bit)
    {
        PRINT("ERR \n");
    }
    GPIOA_ResetBits(need_reset_bit);
    /* 配置唤醒源为 GPIO _ */
    GPIOA_ClearITFlagBit( EC_A|EC_B|SPI_MOTION);
    GPIOB_ClearITFlagBit( KEY_R_0|KEY_R_1|KEY_R_2|KEY_R_3);

    GPIOA_ITModeCfg( SPI_MOTION, GPIO_ITMode_LowLevel );
//    GPIOB_ITModeCfg( KEY_R_0|KEY_R_1|KEY_R_2|KEY_R_3, GPIO_ITMode_LowLevel ); // 下降沿唤醒
    GPIOB_ITModeCfg( KEY_R_0|KEY_R_1|KEY_R_2, GPIO_ITMode_LowLevel ); // 下降沿唤醒
    if(EC_A_ST)
        GPIOA_ITModeCfg( EC_A, GPIO_ITMode_LowLevel );
    else
    {
        GPIOA_ModeCfg(EC_A ,GPIO_ModeIN_PD);
        GPIOA_ITModeCfg( EC_A, GPIO_ITMode_HighLevel );
    }
    if(EC_B_ST)
        GPIOA_ITModeCfg( EC_B, GPIO_ITMode_LowLevel );
    else
    {
        GPIOA_ModeCfg(EC_B ,GPIO_ModeIN_PD);
        GPIOA_ITModeCfg( EC_B, GPIO_ITMode_HighLevel );
    }
    GPIOA_ClearITFlagBit( 0xFFFF );
    GPIOB_ClearITFlagBit( 0xFFFF );
    if(!GPIOA_ReadPortPin(SPI_MOTION))
    {
        PRINT("ERR 1 \n");
    }

//    GPIOA_ModeCfg(GPIO_Pin_All ,GPIO_ModeIN_PU);
//    GPIOB_ModeCfg(GPIO_Pin_All&(~GPIO_Pin_23) ,GPIO_ModeIN_PU);
//    pin_state = GPIOA_ReadPort();
//    PRINT("s1 0x%x\n",~pin_state);
//    GPIOA_ModeCfg(~pin_state ,GPIO_ModeIN_PD);
//    pin_state = GPIOB_ReadPort();
//    PRINT("s2 0x%x  ???\n",~pin_state);
//    GPIOB_ModeCfg((~pin_state)&(~GPIO_Pin_23) ,GPIO_ModeIN_PD);
//    GPIOB_ModeCfg(GPIO_Pin_14 ,GPIO_ModeIN_PD);

    GPIOA_ModeCfg(VBAT_ADC_PIN ,GPIO_ModeIN_PD);
    enter_sleep_flag = TRUE;
    PFIC_ClearPendingIRQ(GPIO_B_IRQn);
    PFIC_EnableIRQ( GPIO_B_IRQn );
    PFIC_ClearPendingIRQ(GPIO_A_IRQn);
    PFIC_EnableIRQ( GPIO_A_IRQn );
//    key_scan_filter_count_max = 0;
    PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_GPIO_WAKE, Long_Delay );
}

/*******************************************************************************
 * @fn      peripheral_exit_sleep
 *
 * @brief   非按键唤醒，可能是收到了上位机数据唤醒
 *
 * @return  None.
 */
void peripheral_exit_sleep()
{
    // 关中断
    access_weakup();
    idel_sleep_flag = FALSE;
    deep_sleep_flag = FALSE;
    enter_sleep_flag = FALSE;
//    key_scan_filter_count_max = KEY_SCAN_FILTER_COUNT_MAX;
    R16_PA_INT_EN = 0;
    R16_PB_INT_EN = 0;
    PWMX_LED_ALL_UPDATE();
    VBAT_CHECK_ENABLE();
    PRINT("exit\n");
}

/*********************************************************************
 * @fn      peripheral_pairing_cb
 *
 * @brief   配对新设备，2HZ快速闪烁
 *
 * @return  none
 */
void peripheral_pairing_cb()
{
    connect_state = CON_NEW_PAIRING;
    peripheral_sleep_update();
    if(nvs_flash_info.led_onoff)
    {
        led_blink_range = LED_BLINK_ALL;
        tmos_clear_event(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT);
        tmos_start_task(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT, PAIRING_LED_BLINK_TIMEOUT);
        tmos_start_reload_task(mouse_taskID, MOUSE_LED_BLINK_EVENT, PAIRING_LED_BLINK_INTERVAL);
    }
}

/*********************************************************************
 * @fn      peripheral_connecting_cb
 *
 * @brief   广播等待连接中。1Hz闪烁
 *
 * @return  none
 */
void peripheral_connecting_cb()
{
    connect_state = CON_CONNECTING;
    peripheral_sleep_update();
    if(nvs_flash_info.led_onoff)
    {
        led_blink_range = LED_BLINK_ALL;
        tmos_clear_event(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT);
        tmos_start_task(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT, DISCONNECT_LED_BLINK_TIMEOUT);
        tmos_start_reload_task(mouse_taskID, MOUSE_LED_BLINK_EVENT, DISCONNECT_LED_BLINK_INTERVAL);
    }
}

/*********************************************************************
 * @fn      peripheral_disconnected_cb
 *
 * @brief   如果BLE或者2.4G断开连接，数字键这一行（对应R1这行）闪烁30s，频率1Hz，则30s后关闭所有背光，进入低功耗Sleep休眠。
 *
 * @return  none
 */
void peripheral_disconnected_cb()
{
    connect_state = CON_IDEL;
    if(!deep_sleep_flag)
    {
        peripheral_sleep_update();
    }
    if(nvs_flash_info.led_onoff)
    {
        led_blink_range = LED_BLINK_ALL;
        tmos_clear_event(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT);
        tmos_start_task(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT, DISCONNECT_LED_BLINK_TIMEOUT);
        tmos_start_reload_task(mouse_taskID, MOUSE_LED_BLINK_EVENT, DISCONNECT_LED_BLINK_INTERVAL);
    }
}

/*********************************************************************
 * @fn      peripheral_connected_cb
 *
 * @brief
 *
 * @return  none
 */
void peripheral_connected_cb()
{
    connect_state = CON_CONNECTED;
    peripheral_sleep_update();
    tmos_set_event(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT);
}

/*********************************************************************
 * @fn      peripheral_pilot_led_receive
 *
 * @brief   peripheral_pilot_led_receive
 *
 * @return  none
 */
void peripheral_pilot_led_receive(uint8_t led)
{

}


/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA中断函数,说明被唤醒了
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler( void )
{
    PRINT("AQ %x\n",R16_PA_INT_IF);
    GPIOA_ClearITFlagBit( 0xFFFF );
    mouse_scan_flag = 1;
    // 停止睡眠
    R16_PA_INT_EN = 0;
    R16_PB_INT_EN = 0;
}

/*********************************************************************
 * @fn      GPIOB_IRQHandler
 *
 * @brief   GPIOB中断函数,说明被唤醒了
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler( void )
{
    PRINT("BQ %x\n",R16_PB_INT_IF);
    GPIOB_ClearITFlagBit( 0xFFFF );
    mouse_scan_flag = 1;
    // 停止睡眠
    R16_PA_INT_EN = 0;
    R16_PB_INT_EN = 0;
}

/*******************************************************************************
 * @fn      mouse_init
 *
 * @brief   mouse_init
 *
 * @return  None.
 */
void mouse_init()
{
    mouse_taskID = TMOS_ProcessEventRegister(mouse_process_event);
    GPIOA_SetBits(KEY_C_0|KEY_C_1);
    GPIOA_ModeCfg(KEY_C_0|KEY_C_1, GPIO_ModeOut_PP_20mA);
    GPIOB_ModeCfg(KEY_R_0|KEY_R_1|KEY_R_2|KEY_R_3 ,GPIO_ModeIN_PU);
//    GPIOB_ModeCfg(VDR_R|VDR_G|VDR_B ,GPIO_ModeOut_PP_20mA);
    GPIOB_SetBits(VDR_R|VDR_G|VDR_B);
    GPIOB_ModeCfg(VDR_R|VDR_G|VDR_B ,GPIO_ModeOut_PP_20mA);

    paw3395_init();
    GPIOA_ResetBits(SPI_CS);
    paw3395_write_byte(PAW_RUN_DOWNSHIFT,0xED);//3s 进rest1
    paw3395_write_byte(PAW_REST_DOWNSHIFT_MULT,0x77);
    paw3395_write_byte(PAW_REST1_DOWNSHIFT,22);//57s 进rest2 223
    paw3395_write_byte(PAW_REST2_DOWNSHIFT,68);//29min 进rest3
    paw3395_write_byte(PAW_RESOLUTION_X_LOW, DPI_VALUE[nvs_flash_info.dpi]&0xFF);
    paw3395_write_byte(PAW_RESOLUTION_X_HIGH, (DPI_VALUE[nvs_flash_info.dpi]&0xFF00)>>8);
    paw3395_write_byte(PAW_RESOLUTION_Y_LOW, DPI_VALUE[nvs_flash_info.dpi]&0xFF);
    paw3395_write_byte(PAW_RESOLUTION_Y_HIGH, (DPI_VALUE[nvs_flash_info.dpi]&0xFF00)>>8);
    paw3395_write_byte(PAW_SET_RESOLUTION, 0x01);
    GPIOA_SetBits(SPI_CS);

    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR0_IRQn);
    TMR0_TimerInit(FREQ_SYS/SAMPLING_RATE+1);
    PFIC_SetPriority(TMR0_IRQn,0xFF);
    tmos_memset(key_data_scan_new, 0, KEY_TABLE_R*KEY_TABLE_C);
    tmos_memset(key_data_scan_old, 0, KEY_TABLE_R*KEY_TABLE_C);
    tmos_memset(key_data_scan_byte, 0, KEY_TABLE_R*KEY_TABLE_C);
    peripheral_sleep_update();

    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    RoughCalib_Value = ADC_DataCalib_Rough(); // 用于计算ADC内部偏差，记录到全局变量 RoughCalib_Value中
    PRINT("ADC RC =%d \n", RoughCalib_Value);

    PWMX_CLKCfg(4);                                    // cycle = 4/Fsys
    PWMX_CycleCfg(PWMX_Cycle_255);                     // 周期 = 255*cycle
    PWMX_ACTOUT(VDR_PWM_R, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_R_IDX], Low_Level, ENABLE);
    PWMX_ACTOUT(VDR_PWM_G, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_G_IDX], Low_Level, ENABLE);
    PWMX_ACTOUT(VDR_PWM_B, DPI_VALUE_RGB[nvs_flash_info.dpi][RGB_B_IDX], Low_Level, ENABLE);
    tmos_set_event(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT);

    if( work_mode != MODE_USB)
    {
        VBAT_CHECK_ENABLE();
    }
}

/*********************************************************************
 * @fn      peripheral_init
 *
 * @brief   peripheral_init
 *
 * @return  none
 */
void peripheral_init()
{
    mouse_init();
    EC10_init();
}

/*********************************************************************
 * @fn      peripheral_process
 *
 * @brief   peripheral_process
 *
 * @return  none
 */
__HIGH_CODE
void peripheral_process()
{
    if(mouse_scan_flag)
    {
        mouse_scan_flag = 0;
        mouse_motion_scan();
    }
}


/*******************************************************************************
 * @fn      mouse_read_xy
 *
 * @brief   mouse_read_xy
 *
 * @return  None.
 */
__HIGH_CODE
uint8_t mouse_read_xy(uint8_t *pX, uint8_t *pY)
{
    uint8_t data[6];
    GPIOA_ResetBits(SPI_CS);
    data[0] = paw3395_read_byte(PAW_MOTION_BURST);
    data[1] = spi_read_byte();
    *pX = spi_read_byte();
     spi_read_byte();
    *pY = spi_read_byte();
     spi_read_byte();
    GPIOA_SetBits(SPI_CS);
    if (!(data[0] & 0x80))
    {
       return 0xFF;
    }
    return 0;
}

/*********************************************************************
 * @fn      mouse_key_scan
 *
 * @brief   mouse_key_scan
 *
 * @return  是否有按键变化
 */
__HIGH_CODE
uint8_t mouse_key_scan(uint8_t *data)
{
    uint8_t ret=0;
    uint8_t i,j,k=0;

    key_press_time++;
    for(i=0; i<KEY_TABLE_C; i++)
    {
        if((idel_sleep_flag||deep_sleep_flag)&&(KEY_C_INDEX[i]==KEY_C_0))   // 中键和拨码开关不支持唤醒，添加判断防止按键检测强制唤醒
        {
            continue;
        }
        GPIOA_ResetBits(KEY_C_INDEX[i]);
        KEY_DETECTE_DELAY();
        for(j=0; j<KEY_TABLE_R; j++)
        {
            k = j+i*KEY_TABLE_R;
            if(!GPIOB_ReadPortPin(KEY_R_INDEX[j]))
            {
//                if(KEY_TABLE[j]<0xF0)
//                    PRINT("k %x\n",KEY_TABLE[k]);
                key_data_scan_new[k] = KEY_TABLE[k];
            }
            else
            {
                key_data_scan_new[k] = 0;
            }
        }
        GPIOA_SetBits(KEY_C_0|KEY_C_1);
    }
    // 过滤
    if(key_scan_filter_count >= key_scan_filter_count_max)
    {
        key_scan_filter_count = 0;
        if(key_scan_filter_count_max)
        {
            if(!tmos_memcmp(key_data_scan_filter, key_data_scan_new, KEY_TABLE_R*KEY_TABLE_C))
            {
                peripheral_sleep_update();
                return ret;
            }
        }
    }
    else
    {
        if(key_scan_filter_count)
        {
            if(!tmos_memcmp(key_data_scan_filter, key_data_scan_new, KEY_TABLE_R*KEY_TABLE_C))
            {
                key_scan_filter_count = 0;
                peripheral_sleep_update();
                return ret;
            }
        }
        tmos_memcpy(key_data_scan_filter, key_data_scan_new, KEY_TABLE_R*KEY_TABLE_C);
        key_scan_filter_count++;
        return ret;
    }
    // 处理数据
    if(!tmos_memcmp(key_data_scan_old, key_data_scan_new, KEY_TABLE_R*KEY_TABLE_C))
    {
        tmos_memcpy(key_data_scan_old, key_data_scan_new, KEY_TABLE_R*KEY_TABLE_C);
        // 先检查当前模式
        if(key_data_scan_new[TABLE_IDX_SW_BLE])
        {
            // SW_BLE
            if( work_mode != MODE_BT)
            {
                PRINT("MODE_BT\n");
                DelayMs(10);
                SYS_ResetExecute();
            }
        }
        else if(key_data_scan_new[TABLE_IDX_SW_24G])
        {
            // SW_24G
            if( work_mode != MODE_2_4G)
            {
                PRINT("MODE_2_4G\n");
                DelayMs(10);
                SYS_ResetExecute();
            }
        }
        else
        {
            if( work_mode != MODE_USB)
            {
                PRINT("MODE_USB\n");
                DelayMs(10);
                SYS_ResetExecute();
            }
        }

        for(i=0; i<KEY_TABLE_R*KEY_TABLE_C; i++)
        {
            if(key_data_scan_new[i] < key_data_scan_byte[i])    // 筛选标准键盘按键
            {
                //释放的按键
                if(key_data_scan_byte[i]<0xE0)
                {
                    // 左中右三个按键
                    data[1] &= ~key_data_scan_byte[i];
                    ret = 1;
                }
                else if(key_data_scan_byte[i]<0xF0)
                {
                    // 侧边两按键
                    data[1] &= ~(1<<(key_data_scan_byte[i]&0x0F));
                    ret = 1;
                }
                else {
                    //DPI 松开
                    // 不考虑循环一轮的情况，太长了
                    if(KEY_TIMER_TABLE[i] )
                    {
                        // 若列表内值为0则表示已经进了超时了，不触发松开
                        if(key_timer_list[KEY_TIMER_TABLE[i]-1])
                        {
                            key_timer_list[KEY_TIMER_TABLE[i]-1] = 0;
                            //短按松开
                            key_release(key_data_scan_byte[i]);
                        }
                    }
                    else
                    {
                        //短按松开
                        key_release(key_data_scan_byte[i]);
                    }
                }
                key_data_scan_byte[i] = 0;
            }
            else if((key_data_scan_new[i] == key_data_scan_byte[i]) && key_data_scan_new[i])
            {
                //未释放的按键，添加到按键列表
                if(key_data_scan_byte[i]<0xE0)
                {
                    // 左中右三个按键
                    data[1] |= key_data_scan_new[i];
                }
                else if(key_data_scan_byte[i]<0xF0)
                {
                    // 侧边两按键 这里演示标准前进后退，如果需要自定义可以动态修改
                    data[1] |= 1<<(key_data_scan_new[i]&0x0F);
                }
                else {
                    //DPI 保持
                }
                key_data_scan_new[i] = 0;
            }
            else if(key_data_scan_new[i])
            {
                // 到这里说明此按键为新按下，添加到列表
                if(key_data_scan_new[i]<0xE0)
                {
                    // 左中右三个按键
                    data[1] |= key_data_scan_new[i];
                    ret = 1;
                }
                else if(key_data_scan_new[i]<0xF0)
                {
                    // 侧边两按键 这里演示标准前进后退，如果需要自定义可以动态修改
                    data[1] |= 1<<(key_data_scan_new[i]&0x0F);
                    ret = 1;
                }
                else
                {
                    //DPI 按下
                    // 触发长按计时器
                    if(KEY_TIMER_TABLE[i])
                    {
                        key_timer_list[KEY_TIMER_TABLE[i]-1] = key_press_time;
                    }
                }
                key_data_scan_byte[i] = key_data_scan_new[i];
            }
        }
    }
    // 长按计时筛选
    for(i=0; i<KEY_TIMEOUT_NUM; i++)
    {
        if(key_timer_list[i])
        {
//            PRINT("%d\n",((key_press_time - key_timer_list[i])/SAMPLING_RATE));
            if(((key_press_time - key_timer_list[i])/SAMPLING_RATE)>KEY_TIMEOUT_LIST[i])
            {
                key_press_timeout(KEY_TIMEOUT_TABLE[i]);
                key_timer_list[i] = 0;
            }
        }
    }
    return ret;
}

extern uint32_t gErrCount;
extern uint32_t gTxCount;
uint32_t gTTXCount;

/*********************************************************************
 * @fn      mouse_motion_scan
 *
 * @brief   模拟鼠标正方形轨迹
 *
 * @return  none
 */
__HIGH_CODE
void mouse_motion_scan(void)
{
    static uint8_t div=0;
    uint8_t motion_div=0;
    uint8_t need_report=0;

    gTxCount++;

    tmos_memset(&mouse_data[2], 0, MOUSE_DATA_LEN-2);
#if 0

    div++;
    if((enter_sleep_flag)&&(!R16_PA_INT_EN)&&(!R16_PB_INT_EN))
    {
        peripheral_sleep_update();
    }
    if(work_mode == MODE_BT)
    {
        motion_div = BLE_MOTION_DIV;
    }
    else
    {
        motion_div = USB_2_4G_MOTION_DIV;
    }

    if((div%motion_div)==0)
    {
        if(!mouse_read_xy(&mouse_data[2],&mouse_data[3]))
        {
            if((mouse_data[2]!=0)||(mouse_data[3]!=0))
            {
//                gErrCount++;
                peripheral_sleep_update();
                need_report = 1;
            }
        }
    }
    if((div%KEY_SCAN_DIV)==0)
    {
        if(mouse_key_scan(mouse_data))
        {
            need_report = 1;
        }
    }
    if((div%WHEEL_DIV)==0)
    {
        mouse_data[4] = EC10_scan();
        if(mouse_data[4])
        {
            need_report = 1;
        }
    }

    if(need_report)
    {
        if(!trans_send_data(mouse_data,MOUSE_DATA_LEN))
        {
            gErrCount++;
        }
    }
#else
    {
        if(gTTXCount<1)
        {
            mouse_data[2] = 0x40;
            mouse_data[3] = 0x00;
        }
        else if(gTTXCount<2)
        {
            mouse_data[2] = 0x00;
            mouse_data[3] = 0xC0;

        }
        else if(gTTXCount<3)
        {
            mouse_data[2] = 0xC0;
            mouse_data[3] = 0x00;

        }
        else if(gTTXCount<4)
        {
            mouse_data[2] = 0x00;
            mouse_data[3] = 0x40;

        }
        if(!trans_send_data(mouse_data,MOUSE_DATA_LEN))
        {
            gTTXCount++;
            gErrCount++;
            if(gTTXCount>=4)
            {
                gTTXCount = 0;
            }
        }
    }
#endif
}

/*********************************************************************
 * @fn      key_press_timeout
 *
 * @brief   key_press_timeout
 *
 * @return  none
 */
void key_press_timeout(uint8_t key)
{
    PRINT("p t %x\n",key);
    if(key == 0xF0) //DPI
    {
        //进入配对
        if(work_mode==MODE_BT)
        {
            access_pairing_process(CTL_MODE_BLE_1);
        }
        else if(work_mode==MODE_2_4G)
        {
            //长按3s后进入2.4G配对状态
            nvs_flash_info.rf_device_id = RF_ROLE_ID_INVALD;
            tmos_memset(nvs_flash_info.peer_mac, 0, 6);
            nvs_flash_store();
            RF_ReStart();
        }
    }
}

/*********************************************************************
 * @fn      key_release
 *
 * @brief   key_release
 *
 * @return  none
 */
void key_release(uint8_t key)
{
//    PRINT("r %x\n",key);
    if(key == 0xF0) //DPI
    {
        //切换DPI
        nvs_flash_info.dpi++;
        nvs_flash_info.dpi %= DPI_MAX;
        nvs_flash_store();
        PRINT("dpi %d\n",DPI_VALUE[nvs_flash_info.dpi]+1*50);
        GPIOA_ResetBits(SPI_CS);
        paw3395_write_byte(PAW_RESOLUTION_X_LOW, DPI_VALUE[nvs_flash_info.dpi]&0xFF);
        paw3395_write_byte(PAW_RESOLUTION_X_HIGH, (DPI_VALUE[nvs_flash_info.dpi]&0xFF00)>>8);
        paw3395_write_byte(PAW_RESOLUTION_Y_LOW, DPI_VALUE[nvs_flash_info.dpi]&0xFF);
        paw3395_write_byte(PAW_RESOLUTION_Y_HIGH, (DPI_VALUE[nvs_flash_info.dpi]&0xFF00)>>8);
        paw3395_write_byte(PAW_SET_RESOLUTION, 0x01);
        GPIOA_SetBits(SPI_CS);
        tmos_set_event(mouse_taskID, MOUSE_LED_TIMEOUT_EVENT);
    }
}


/*********************************************************************
 * @fn      mouse_get_batt_info
 *
 * @brief   电源信息
 *
 * @return  none
 */
uint8_t mouse_get_batt_info()
{
    uint16_t adcBuff[4];
    uint8_t ret;                                   //电量百分比
    uint8_t powerper;                                   //电量百分比
    static uint8_t lastper=0;                                   //上次电量百分比
    static uint8_t lastper_1=0;                                   //上2次电量百分比
    static uint8_t lastper_2=0;                                   //上3次电量百分比
    static uint8_t lastper_3=0;                                   //上4次电量百分比

                                                        //ADC采样通道需要和ADC引脚对应
    GPIOA_ModeCfg( VBAT_ADC_PIN, GPIO_ModeIN_Floating );  //浮空输入模式
    ADC_ChannelCfg( VBAT_ADC_CHANNAL );

    ADC_ExtSingleChSampInit( SampleFreq_3_2, ADC_PGA_0 );
    for(uint8_t i = 0; i < 4; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver() + RoughCalib_Value; // 连续采样
    }
    for(uint8_t i = 0; i < 4; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver() + RoughCalib_Value; // 连续采样
    }

//    PRINT("电压： %d %d %d %d\n",adcBuff[0],adcBuff[1],adcBuff[2],adcBuff[3]);
    adcBuff[0] = (adcBuff[0]+adcBuff[1]+adcBuff[2]+adcBuff[3]+2)/4;

    adcBuff[0] = (uint32_t)adcBuff[0] * 1050 / 2048;                        //测量点电压
//    PRINT("电阻电压： %d\n",adcBuff[0]);

    //电池电压
    adcBuff[0] = (uint32_t)adcBuff[0] * 3 / 1;                           //电池电压
//    PRINT("电池电压： %d\n",adcBuff[0]);
    if( adcBuff[0] <= 3000 )
    {
        powerper = 0;
    }
    else if( adcBuff[0] <= 3200 && adcBuff[0] > 3000 )
    {
        for( uint8_t i = 0; i <= 200; i += 40 )
        {
            if( adcBuff[0] < 3000 + i )
            {
                powerper = i/40;
                break;
            }
        }
    }
    else
    {
        uint8_t i;
        for( i = 6; i <= 100; i++ )
        {
            if( adcBuff[0] < 3200 + i * 9 )
            {
                powerper = i;
                break;
            }
        }
        if( i > 100 )
        {
            powerper = 100;
        }
    }
//    PRINT("powerper： %d\n",powerper);
    if(lastper > 1)
    {
        powerper = (lastper+powerper+1)/2;
    }
    if(lastper_3!= 0)
    {
        ret = (lastper_3+lastper_2+lastper_1+lastper+powerper+2)/5;
    }
    else {
        ret = powerper;
    }
    lastper_3 = lastper_2;
    lastper_2 = lastper_1;
    lastper_1 = lastper;
    lastper = powerper;
//    PRINT("池子： %d %d %d %d %d\n",lastper_3,lastper_2,lastper_1,lastper,powerper);
//    PRINT("电量： %d\n",ret);

    return ret;
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   TMR0_IRQHandler 发送时间等于 200us+8us+发送长度*4us
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void)
{
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
    }
    mouse_scan_flag = 1;
}

/*********************************************************************
 * @fn      PWM_RECOVER
 *
 * @brief   PWM_RECOVER
 *
 * @return  none
 */
void PWM_RECOVER()
{
    if(R32_PWM_RAM_SAFE_FLAG == PWM_RAM_SAFE_FLAG)
    {
        GPIOB_ModeCfg(VDR_R|VDR_G|VDR_B ,GPIO_ModeOut_PP_20mA);
        R32_PWM_CONTROL = R32_PWM_CONTROL_RAM;
        R32_PWM4_7_DATA = R32_PWM4_7_DATA_RAM;
        R32_PWM8_11_DATA = R32_PWM8_11_DATA_RAM;
    }
}

/*********************************************************************
 * @fn      PWM_SAVE
 *
 * @brief   PWM_SAVE
 *
 * @return  none
 */
void PWM_SAVE()
{
    R32_PWM_RAM_SAFE_FLAG = PWM_RAM_SAFE_FLAG;
    R32_PWM_CONTROL_RAM = R32_PWM_CONTROL;
    R32_PWM4_7_DATA_RAM = R32_PWM4_7_DATA;
    R32_PWM8_11_DATA_RAM = R32_PWM8_11_DATA;
}

/******************************** endfile @rf ******************************/
