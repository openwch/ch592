/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_tmos.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/8/5
 * Description        : ������������
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "Touch.h"
#include "CONFIG.h"
#include "app_tmos.h"
#include "peripheral.h"
/*********************
 *      DEFINES
 *********************/
#define SLEEP_TRIGGER_TIME MS1_TO_SYSTEM_TIME(500) // 500ms
#define TRIGGER_TIME MS1_TO_SYSTEM_TIME(100)       // 100ms
#define WAKEUP_TIME MS1_TO_SYSTEM_TIME(5)          // 50ms

/**********************
 *      VARIABLES
 **********************/
tmosTaskID TouchKey_TaskID = 0x00;
uint16_t triggerTime = SLEEP_TRIGGER_TIME;

static const uint8_t touch_key_ch[ TOUCH_KEY_ELEMENTS ] = {TOUCH_KEY_CHS};
KEY_T s_tBtn[TOUCH_KEY_ELEMENTS] = {0};

touch_button_cfg_t p_selfkey =
{
    .num_elements = TOUCH_KEY_ELEMENTS,
    .p_elem_index = touch_key_ch,
    .p_stbtn = s_tBtn
};

touch_cfg_t touch_cfg =
{
    .touch_button_cfg = &p_selfkey,
    .touch_slider_cfg = NULL,
    .touch_wheel_cfg = NULL
};
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void TKY_PeripheralInit(void);
static void peripherals_EnterSleep(void);
static void peripherals_WakeUp(void);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************************************************************
 * @fn      tky_on_TMOS_dataProcess
 *
 * @brief   �������ݴ�����������TMOS�����������ӳɹ��󽫻�ȡ����ֵ��֪ͨ����ʽ�ϱ�����λ������
 *
 * @return  none
 */
void tky_on_TMOS_dataProcess(void)
{
    uint8_t key_val = 0;
    key_val = touch_GetKey();
    if (key_val != 0x00)
    {
        if (bleConnectState )
        {
            peripheralChar2Notify( &key_val, 1 );//����ֵ�ϱ�����λ������
        }
    }
}


/*********************************************************************
 * @fn      PeriodicDealData
 *
 * @brief    ��������״̬����
 *
 * @return  none
 */
void PeriodicDealData(void)
{
    TKY_LoadAndRun(); //---��������ǰ����Ĳ�������---
//    GPIOTK_PinSleep(  );

    //---����̬������ʱ�����л���ʾ���ݡ������߻����ֵ��ÿ���д���ʱ������10��wakeupʱ�䣬���˶�ʱ������ʱ��Ϊ5s---
    if (wakeUpCount)
    {
        wakeUpCount--;
//        dg_log("wakeUpCount: :%d\n", wakeUpCount);
        //---wakeUpCount����Ϊ0������̬����ת����---
        if (wakeUpCount == 0)
        {
        	touch_ScanEnterSleep();

            tmos_stop_task(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT);
            triggerTime = SLEEP_TRIGGER_TIME;
            /*-------------------------
             * Call your peripherals sleep function
             * -----------------------*/
            peripherals_EnterSleep();
        }
    }
    else //---����״̬ʱ�������������ɨ��---
    {
        dg_log("wake up...\n");

        scanData = TKY_ScanForWakeUp(tkyPinAll.tkyQueueAll); //---����ѡ��Ķ���ͨ������ɨ��---

        if (scanData) //---��ɨ�����쳣���������ʽɨ�躯��ģʽ3~4---
        {
            TKY_SetSleepStatusValue((uint16_t)((~scanData)& tkyPinAll.tkyQueueAll)); //---��������״̬�������쳣״̬��ͨ������Ϊ������̬---
            for (uint8_t i = 0; i < 40; i++) //---����һ��Ҫɨ��64�Σ�20�����ϽԿɣ���������������е�ɨ���а������£����˳�ѭ������������ɨ��---
            {
                keyData = TKY_PollForFilter();
                if (keyData) //---һ����⵽�а������£����˳�ѭ��ɨ��---
                {
                	touch_ScanWakeUp();

                    triggerTime = TRIGGER_TIME;
                    tky_DealData_start();
                    tmos_start_task(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT, 0);
                    /*-------------------------
                     * Call your peripherals WakeUp function
                     * -----------------------*/
                    peripherals_WakeUp();
                    break;
                }
            }
        }
    }
    TKY_SaveAndStop(); //---����ؼĴ������б���---
}


/*********************************************************************
 * @fn      tky_DealData_start
 *
 * @brief   ����ɨ�迪������
 *
 * @return  none
 */
void tky_DealData_start(void)
{
    tmos_set_event(TouchKey_TaskID, DEALDATA_EVT);
}

/*********************************************************************
 * @fn      tky_DealData_stop
 *
 * @brief   ����ɨ��ֹͣ����
 *
 * @return  none
 */
void tky_DealData_stop(void)
{
    tmos_stop_task(TouchKey_TaskID, DEALDATA_EVT);
}


/*********************************************************************
 * @fn      Touch_Key_ProcessEvent
 *
 * @brief   ��������������
 *
 * @return  none
 */
tmosEvents Touch_Key_ProcessEvent(tmosTaskID task_id, tmosEvents events)
{
    uint16_t res;

    if (events & WAKEUP_DATA_DEAL_EVT)
    {
        touch_Scan();
        tky_on_TMOS_dataProcess();
#if TKY_SLEEP_EN
        if (wakeupflag)
#endif
            tmos_start_task(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT, WAKEUP_TIME);
        return (events ^ WAKEUP_DATA_DEAL_EVT);
    }

    if (events & DEALDATA_EVT)
    {
        PeriodicDealData();
#if TKY_SLEEP_EN
        if (!advState || wakeupflag)
#endif
            tmos_start_task(TouchKey_TaskID, DEALDATA_EVT, triggerTime);
        return (events ^ DEALDATA_EVT);
    }

#if PRINT_EN
    if (events & DEBUG_PRINT_EVENT)
    {
        touch_InfoDebug();

        tmos_start_task(TouchKey_TaskID, DEBUG_PRINT_EVENT, SLEEP_TRIGGER_TIME);
        return (events ^ DEBUG_PRINT_EVENT);
    }
#endif

    if(events & TKY_KEEPALIVE_EVENT)
    {
        return events;
    }


    return 0;
}


/*********************************************************************
 * @fn      touch_on_TMOS_init
 *
 * @brief   ������ʼ������������TMOS��
 *
 * @return  none
 */
void touch_on_TMOS_init(void)
{
    TouchKey_TaskID = TMOS_ProcessEventRegister(Touch_Key_ProcessEvent);
    TKY_PeripheralInit();       /* ��ʼ���裬���米��ͷ������� */
    touch_Init(&touch_cfg);             /* ��ʼ��������  */

    wakeUpCount = 50; //---����ʱ��---
    wakeupflag = 1;   // �óɻ���״̬
    triggerTime = TRIGGER_TIME;
    TKY_SetSleepStatusValue(~tkyPinAll.tkyQueueAll);
#if TKY_SLEEP_EN
    tky_DealData_start();
#else
    tky_DealData_stop();
#endif

#if PRINT_EN
    tmos_set_event(TouchKey_TaskID, DEBUG_PRINT_EVENT);
#endif

    tmos_set_event(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT);
    tmos_set_event(TouchKey_TaskID, TKY_KEEPALIVE_EVENT);
    dg_log("Touch Key init Finish!\n");
}


/**********************
 *   STATIC FUNCTIONS
 **********************/


/*********************************************************************
 * @fn      TKY_PeripheralInit
 *
 * @brief   ���������ʼ�����������ڳ�ʼ���봥��������ص����蹦��
 *
 * @return  none
 */
static void TKY_PeripheralInit(void)
{
    /*You code here*/
}

/*********************************************************************
 * @fn      peripherals_EnterSleep
 *
 * @brief   ����˯�ߺ������ڴ���׼������ʱ����
 *
 * @return  none
 */
static void peripherals_EnterSleep(void)
{
    /*You code here*/
    tmos_stop_task(TouchKey_TaskID, TKY_KEEPALIVE_EVENT);
}


/*********************************************************************
 * @fn      peripherals_WakeUp
 *
 * @brief   ���軽�Ѻ������ڴ���������ʱ����
 *
 * @return  none
 */
static void peripherals_WakeUp(void)
{
    /*You code here*/
    tmos_set_event(TouchKey_TaskID, TKY_KEEPALIVE_EVENT);
}
