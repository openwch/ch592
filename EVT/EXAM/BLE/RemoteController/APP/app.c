/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : APP应用演示，包含按键检测，低功耗睡眠唤醒，ADC电量采集，PWM驱灯
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "battservice.h"
#include "hiddev.h"
#include "hidkbd.h"
#include "app.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define KEY_SCAN_ENABLE             TRUE

#define BATT_COLLECT_ENABLE         TRUE
#define VBAT_INFO_DEBUG             TRUE

#define LOWPOWER_SLEEP_ENABLE       TRUE

#define PWM_LED_ENABLE              TRUE

// Task ID
static uint8_t appTaskId = INVALID_TASK_ID;
uint32_t key_scan_time = 0;
uint32_t key_data = 0;
uint32_t key_timer_list[KEY_NUM];

uint8_t batt_val_percent;
signed short RoughCalib_Value = 0; // ADC粗调偏差值

uint8_t app_sleep_flag = FALSE;
uint8_t app_pair_flag = FALSE;

/* Set by GPIOB ISR, cleared by App_GpioWakeupHandle() in main loop */
volatile uint8_t app_gpio_wakeup_flag = FALSE;

uint8_t led_duty_cycle = 0;

/*********************************************************************
 * @fn      App_Init
 *
 * @brief   App 初始化
 *
 * @return  none
 */
void App_Init(void)
{
    uint8_t bond_count;
    appTaskId = TMOS_ProcessEventRegister(App_ProcessEvent);

#if KEY_SCAN_ENABLE
    {
        GPIOB_ModeCfg(KEY_BOOT, GPIO_ModeIN_PU);
        tmos_start_reload_task(appTaskId, APP_KEYSCAN_EVT, APP_KEYSCAN_INT);
    }
#endif
#if BATT_COLLECT_ENABLE
    {
        ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
        RoughCalib_Value = ADC_DataCalib_Rough(); // 用于计算ADC内部偏差，记录到全局变量 RoughCalib_Value中
        PRINT("ADC RC =%d \n", RoughCalib_Value);
        // 建议在连接之前就设初始值，否侧刚连上会随机显示值。
        App_GetBattInfo();
        App_GetBattInfo();
        App_GetBattInfo();
        App_GetBattInfo();
        batt_val_percent = App_GetBattInfo();
        Batt_SetParameter(BATT_PARAM_CRITICAL_LEVEL, sizeof(uint8_t), &batt_val_percent);
        tmos_start_reload_task(appTaskId, APP_BATT_EVT, APP_BATT_TIMEOUT);
    }
#endif
#if PWM_LED_ENABLE
    {
        GPIOB_ModeCfg(LED_PIN, GPIO_ModeIN_PU);         // PB14 - PWM10
        PWMX_CLKCfg(4);                                 // cycle = 4/Fsys
        PWMX_CycleCfg(PWMX_Cycle_64);                   // 周期 = 64*cycle
    }
#endif
    // 如果查不到绑定信息，则打开配对广播
    GAPBondMgr_GetParameter(GAPBOND_BOND_COUNT, &bond_count);
    if(!bond_count)
    {
        app_pair_flag = TRUE;
    }
    hidEmu_adv_enable(ENABLE);
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
    GPIOB_ClearITFlagBit( 0xFFFF );
    // 关闭中断
    R16_PB_INT_EN = 0;
    /* Set flag only; tmos_set_event is invoked from main loop context */
    app_gpio_wakeup_flag = TRUE;
}

/*********************************************************************
 * @fn      App_GpioWakeupHandle
 *
 * @brief   Convert GPIO wake flag into TMOS event from main loop
 *
 * @return  none
 */
void App_GpioWakeupHandle(void)
{
    if(app_gpio_wakeup_flag)
    {
        app_gpio_wakeup_flag = FALSE;
        tmos_set_event(appTaskId, APP_WEAK_UP_EVT);
    }
}

/*********************************************************************
 * @fn      App_Update_Sleep
 *
 * @brief   刷新进入空闲，睡眠和深度休眠的超时时间
 *
 * @return  none
 */
void App_Update_Sleep(void)
{
#if LOWPOWER_SLEEP_ENABLE
    uint8_t ble_state;
    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
    if( ble_state == GAPROLE_CONNECTED )
    {
        tmos_start_task(appTaskId, APP_ENTER_SLEEP_EVT, APP_CON_SLEEP_TIMEOUT);
    }
    else
    {
        tmos_start_task(appTaskId, APP_ENTER_SLEEP_EVT, APP_ADV_SLEEP_TIMEOUT);
    }
    tmos_start_task(appTaskId, APP_ENTER_IDEL_EVT, APP_CON_IDEL_TIMEOUT);
    tmos_start_task(appTaskId, APP_SHUTDOWN_EVT, APP_SHUTDOWN_TIMEOUT);
#endif
}

/*********************************************************************
 * @fn      App_Enter_Idel
 *
 * @brief   进入空闲模式，保持连接，停止键扫，通过按键中断触发唤醒
 *
 * @return  none
 */
void App_Enter_Idel(void)
{
    PRINT( "Enter_Idel\n");
    tmos_stop_task(appTaskId, APP_KEEP_WEAK_EVT);
    tmos_stop_task(appTaskId, APP_KEYSCAN_EVT);
    GPIOB_ITModeCfg(KEY_BOOT, GPIO_ITMode_LowLevel);
    GPIOB_ClearITFlagBit( 0xFFFF );
    PFIC_EnableIRQ( GPIO_B_IRQn );
    PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_GPIO_WAKE, Long_Delay );
}

/*********************************************************************
 * @fn      App_Enter_Sleep
 *
 * @brief   进入低功耗模式，断开连接，停止广播，通过按键触发唤醒
 *
 * @return  none
 */
void App_Enter_Sleep(void)
{
    uint8_t ble_state;
    app_sleep_flag = TRUE;
    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
    PRINT( "Enter_Sleep state %x\n", ble_state );
    if( ble_state == GAPROLE_CONNECTED )
    {
        GAPRole_TerminateLink(hidEmuConnHandle);
    }
    else if( ble_state == GAPROLE_ADVERTISING )
    {
        hidEmu_adv_enable(DISABLE);
    }
    tmos_stop_task(appTaskId, APP_KEEP_WEAK_EVT);
    tmos_stop_task(appTaskId, APP_KEYSCAN_EVT);
#if BATT_COLLECT_ENABLE
    tmos_start_reload_task(appTaskId, APP_BATT_EVT, APP_BATT_TIMEOUT*30);
#endif
    GPIOB_ITModeCfg(KEY_BOOT, GPIO_ITMode_LowLevel);
    GPIOB_ClearITFlagBit( 0xFFFF );
    PFIC_EnableIRQ( GPIO_B_IRQn );
    PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_GPIO_WAKE, Long_Delay );
}

/*********************************************************************
 * @fn      App_WeakUp
 *
 * @brief   恢复正常工作模式
 *
 * @return  none
 */
void App_WeakUp(void)
{
    uint8_t ble_state;
    uint8_t bond_count;
    app_sleep_flag = FALSE;
    GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
    PRINT( "WeakUp state %x\n", ble_state );
    if( (ble_state != GAPROLE_ADVERTISING) &&  (ble_state != GAPROLE_CONNECTED))
    {
        GAPBondMgr_GetParameter(GAPBOND_BOND_COUNT, &bond_count);
        if(!bond_count)
        {
            app_pair_flag = TRUE;
        }
        else
        {
            app_pair_flag = FALSE;
        }
        hidEmu_adv_enable(ENABLE);
    }
    tmos_start_reload_task(appTaskId, APP_KEYSCAN_EVT, APP_KEYSCAN_INT);
#if BATT_COLLECT_ENABLE
    tmos_start_reload_task(appTaskId, APP_BATT_EVT, APP_BATT_TIMEOUT);
#endif
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
uint16_t App_ProcessEvent(uint8_t task_id, uint16_t events)
{

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(appTaskId)) != NULL)
        {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & APP_BATT_EVT)
    {
        batt_val_percent = App_GetBattInfo();
        Batt_SetParameter(BATT_PARAM_CRITICAL_LEVEL, sizeof(uint8_t), &batt_val_percent);
        return (events ^ APP_BATT_EVT);
    }

    if(events & APP_WEAK_UP_EVT)
    {
        App_WeakUp();
        return (events ^ APP_WEAK_UP_EVT);
    }

    if(events & APP_SHUTDOWN_EVT)
    {
        LowPower_Shutdown(0);
        return (events ^ APP_SHUTDOWN_EVT);
    }

    if(events & APP_ENTER_SLEEP_EVT)
    {
        App_Enter_Sleep();
        return (events ^ APP_ENTER_SLEEP_EVT);
    }

    if(events & APP_ENTER_IDEL_EVT)
    {
        App_Enter_Idel();
        return (events ^ APP_ENTER_IDEL_EVT);
    }

    if(events & APP_KEYSCAN_EVT)
    {
        Key_Scan();
        return (events ^ APP_KEYSCAN_EVT);
    }

    if(events & APP_LED_TIMEOUT_EVT)
    {
        GPIOB_ModeCfg(LED_PIN, GPIO_ModeIN_PU);
        tmos_stop_task(appTaskId, APP_LED_UPDATE_EVT);
        tmos_stop_task(appTaskId, APP_KEEP_WEAK_EVT);
        return (events ^ APP_LED_TIMEOUT_EVT);
    }

    if(events & APP_LED_UPDATE_EVT)
    {
        App_LedUpdate();
        return (events ^ APP_LED_UPDATE_EVT);
    }

    if(events & APP_KEEP_WEAK_EVT)
    {
        // 必须放在任务列表的最后
        // 用于限制tmos任务管理的睡眠，当有任务存在时，蓝牙库不会调用睡眠
        return (events);
    }
    return 0;
}

/*********************************************************************
 * @fn      App_LedUpdate
 *
 * @brief   刷新led
 *
 * @return  none
 */
void App_LedUpdate()
{
    led_duty_cycle++;
    if(led_duty_cycle>=128)
        led_duty_cycle=0;
    PWMX_ACTOUT(LED_PWM_ID, (led_duty_cycle<64)?led_duty_cycle:(128-led_duty_cycle), Low_Level, ENABLE); // 25% 占空比
}

/*********************************************************************
 * @fn      App_LedBreathStart
 *
 * @brief   启动led呼吸灯,2秒后停止
 *
 * @return  none
 */
void App_LedBreathStart()
{
    GPIOB_ModeCfg(LED_PIN, GPIO_ModeOut_PP_5mA);
    led_duty_cycle = 0;
    PWMX_ACTOUT(LED_PWM_ID, led_duty_cycle, Low_Level, ENABLE); // 25% 占空比
    tmos_start_reload_task(appTaskId, APP_LED_UPDATE_EVT, APP_LED_UPDATE_INT);
    tmos_start_task(appTaskId, APP_LED_TIMEOUT_EVT, APP_LED_UTIMEOUT);
    // 用于限制tmos任务管理的睡眠，当有任务存在时，蓝牙库不会调用睡眠，睡眠会导致PWM停止
    tmos_set_event(appTaskId, APP_KEEP_WEAK_EVT);
}

/*********************************************************************
 * @fn      Key_Press_CB
 *
 * @brief   按键短按
 *
 * @return  none
 */
void Key_Press_CB(uint8_t key_idx)
{
    uint8_t report_buffer[8];
    App_Update_Sleep();
    switch(key_idx)
    {
        case KEY_BOOT_ID:
        {
            //短按BOOT按键,发送静音按键,同时led灯呼吸一次
            report_buffer[0] = 0xe2;
            report_buffer[1] = 0;
            hidEmu_consumer_report(report_buffer, 2);
            report_buffer[0] = 0;
            hidEmu_consumer_report(report_buffer, 2);
#if PWM_LED_ENABLE
            App_LedBreathStart();
#endif
        }
    }
}

/*********************************************************************
 * @fn      Key_Press_Timeout_CB
 *
 * @brief   按键长按
 *
 * @return  none
 */
void Key_Press_Timeout_CB(uint8_t key_idx)
{
    App_Update_Sleep();
    switch(key_idx)
    {
        case KEY_BOOT_ID:
        {
            //长按BOOT按键，进配对模式
            uint8_t ble_state;
            app_pair_flag = TRUE;
            GAPRole_GetParameter( GAPROLE_STATE, &ble_state );
            if( ble_state == GAPROLE_CONNECTED )
            {
                GAPRole_TerminateLink(hidEmuConnHandle);
            }
            else if( ble_state == GAPROLE_ADVERTISING )
            {
                hidEmu_adv_enable(DISABLE);
            }
            else
            {
                hidEmu_adv_enable(ENABLE);
            }
        }
    }
}

/*********************************************************************
 * @fn      key_scan
 *
 * @brief   App 初始化
 *
 * @return  none
 */
void Key_Scan(void)
{
    uint32_t key_data_scan = 0;
    uint8_t i;
    if(!GPIOB_ReadPortPin(KEY_BOOT))
    {
        key_data_scan |= KEY_DATA_BOOT;
    }
    key_scan_time++;
    if(key_data_scan != key_data)
    {
        for(i=0; i<KEY_NUM; i++)
        {
            if((key_data_scan&(1<<i)) == (key_data&(1<<i)))
            {
                // 维持不变的按键
            }
            else if((key_data_scan&(1<<i)) > (key_data&(1<<i)))
            {
                // 新按下的按键
                key_timer_list[i] = key_scan_time;
            }
            else
            {
                // 释放的按键
                if(key_timer_list[i])
                {
                    // 没有触发超时就松开，属于短按
                    Key_Press_CB(i);
                }
                key_timer_list[i] = 0;
            }
        }
        key_data = key_data_scan;
    }

    //
    for(i=0; i<KEY_NUM; i++)
    {
        if(key_timer_list[i])
        {
            if(((key_scan_time - key_timer_list[i])/(1600/APP_KEYSCAN_INT))>KEY_TIMEOUT)
            {
                Key_Press_Timeout_CB(i);
                key_timer_list[i] = 0;
            }
        }
    }

}

/*********************************************************************
 * @fn      App_GetBattInfo
 *
 * @brief   获取电源信息
 *
 * @return  none
 */
uint8_t App_GetBattInfo()
{
    uint16_t adcBuff[4];
    uint8_t ret;                                   //电量百分比
    uint8_t powerper;                                   //电量百分比
    static uint8_t lastper=0;                                   //上次电量百分比
    static uint8_t lastper_1=0xFF;                                   //上2次电量百分比
    static uint8_t lastper_2=0xFF;                                   //上3次电量百分比
    static uint8_t lastper_3=0xFF;                                   //上4次电量百分比

                                                        //ADC采样通道需要和ADC引脚对应
    GPIOA_ModeCfg( VBAT_ADC_PIN, GPIO_ModeIN_Floating );  //浮空输入模式
    ADC_ChannelCfg( VBAT_ADC_CHANNAL );

    ADC_ExtSingleChSampInit( SampleFreq_3_2, ADC_PGA_0 );
    // 前四次采样丢弃，如果刚唤醒时采样可能不准
    for(uint8_t i = 0; i < 4; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver() + RoughCalib_Value; // 连续采样
    }
    for(uint8_t i = 0; i < 4; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver() + RoughCalib_Value; // 连续采样
    }
#if VBAT_INFO_DEBUG
    PRINT("Vol %d %d %d %d\n",adcBuff[0],adcBuff[1],adcBuff[2],adcBuff[3]);
#endif
    adcBuff[0] = (adcBuff[0]+adcBuff[1]+adcBuff[2]+adcBuff[3]+2)/4;

    adcBuff[0] = (uint32_t)adcBuff[0] * 1050 / 2048;
#if VBAT_INFO_DEBUG                    //测量点电压
    PRINT("Resistance vol %d\n",adcBuff[0]);
#endif

    //这里模拟电池电压与测量电阻分压比例为3:1
    adcBuff[0] = (uint32_t)adcBuff[0] * 3 / 1;
#if VBAT_INFO_DEBUG                       //电池电压
    PRINT("Batt vol %d\n",adcBuff[0]);
#endif
    // 一种分为两段的线性计算电量的方式
    if( adcBuff[0] <= 3000 )
    {
        powerper = 0;
    }
    else if( adcBuff[0] <= 3300 && adcBuff[0] > 3000 )
    {
        for( uint16_t i = 0; i <= 300; i += 60 )
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
            if( adcBuff[0] < 3300 + i * 9 )
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
#if VBAT_INFO_DEBUG
    PRINT("powerper %d\n",powerper);
#endif
    // 滤波
    if(lastper > 1)
    {
        powerper = (lastper+powerper+1)/2;
    }
    if(lastper_3!= 0xFF)
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
#if VBAT_INFO_DEBUG
    PRINT("Batt pool %d %d %d %d %d\n",lastper_3,lastper_2,lastper_1,lastper,powerper);
    PRINT("Batt %d\n",ret);
#endif

    return ret;
}

/******************************** endfile @ main ******************************/
