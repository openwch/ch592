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
#include "app.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      VARIABLES
 **********************/
UINT8V timerFlag = 0;



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

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************************************************************
 * @fn      TKY_dataProcess
 *
 * @brief   �������ݴ����������ܣ�����ӡ��ȡ���İ����������
 *
 * @return  none
 */
void TKY_dataProcess(void)
{
    uint8_t key_val = 0;
    static uint16_t print_time = 0;

    if(timerFlag)
    {
        timerFlag = 0;
        touch_Scan();
#if PRINT_EN
        print_time++;
        if(print_time == 500)
        {
            print_time = 0;
            touch_InfoDebug();
        }
#endif
    }
    key_val = touch_GetKey();
    switch(key_val)
    {
       case KEY_NONE   :   break;
       case KEY_0_DOWN :   PRINT("KEY_1_DOWN !\n");break;
       case KEY_0_UP   :   PRINT("KEY_1_UP   !\n");break;
       case KEY_0_LONG :   PRINT("KEY_1_LONG !\n");break;
       case KEY_1_DOWN :   PRINT("KEY_2_DOWN !\n");break;
       case KEY_1_UP   :   PRINT("KEY_2_UP   !\n");break;
       case KEY_1_LONG :   PRINT("KEY_2_LONG !\n");break;
       case KEY_2_DOWN :   PRINT("KEY_3_DOWN !\n");break;
       case KEY_2_UP   :   PRINT("KEY_3_UP   !\n");break;
       case KEY_2_LONG :   PRINT("KEY_3_LONG !\n");break;
       case KEY_3_DOWN :   PRINT("KEY_4_DOWN !\n");break;
       case KEY_3_UP   :   PRINT("KEY_4_UP   !\n");break;
       case KEY_3_LONG :   PRINT("KEY_4_LONG !\n");break;
       case KEY_4_DOWN :   PRINT("KEY_5_DOWN !\n");break;
       case KEY_4_UP   :   PRINT("KEY_5_UP   !\n");break;
       case KEY_4_LONG :   PRINT("KEY_5_LONG !\n");break;
       case KEY_5_DOWN :   PRINT("KEY_6_DOWN !\n");break;
       case KEY_5_UP   :   PRINT("KEY_6_UP   !\n");break;
       case KEY_5_LONG :   PRINT("KEY_6_LONG !\n");break;
       case KEY_6_DOWN :   PRINT("KEY_7_DOWN !\n");break;
       case KEY_6_UP   :   PRINT("KEY_7_UP   !\n");break;
       case KEY_6_LONG :   PRINT("KEY_7_LONG !\n");break;
       case KEY_7_DOWN :   PRINT("KEY_8_DOWN !\n");break;
       case KEY_7_UP   :   PRINT("KEY_8_UP   !\n");break;
       case KEY_7_LONG :   PRINT("KEY_8_LONG !\n");break;
       case KEY_8_DOWN :   PRINT("KEY_9_DOWN !\n");break;
       case KEY_8_UP   :   PRINT("KEY_9_UP   !\n");break;
       case KEY_8_LONG :   PRINT("KEY_9_LONG !\n");break;
       case KEY_9_DOWN :   PRINT("KEY_*_DOWN !\n");break;
       case KEY_9_UP   :   PRINT("KEY_*_UP   !\n");break;
       case KEY_9_LONG :   PRINT("KEY_*_LONG !\n");break;
       case KEY_10_DOWN :  PRINT("KEY_0_DOWN!\n");break;
       case KEY_10_UP  :   PRINT("KEY_0_UP  !\n");break;
       case KEY_10_LONG :  PRINT("KEY_0_LONG!\n");break;
       case KEY_11_DOWN :  PRINT("KEY_#_DOWN!\n");break;
       case KEY_11_UP  :   PRINT("KEY_#_UP  !\n");break;
       case KEY_11_LONG :  PRINT("KEY_#_LONG!\n");break;
       default : break;
    }
}


/*********************************************************************
 * @fn      touch_init
 *
 * @brief   ������ʼ����������ʹ��tmos����Ҫ�豸������ʱ����
 *
 * @return  none
 */
void TKY_Init(void)
{
	TKY_PeripheralInit();       /* ��ʼ�����裬���米��ͷ������� */

    touch_Init(&touch_cfg);             /* ��ʼ��������  */

    TKY_SetSleepStatusValue( ~tkyPinAll.tkyQueueAll );

    TMR0_TimerInit(FREQ_SYS/1000);               //��ʱ����Ϊ1ms
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ( TMR0_IRQn );

    dg_log("Touch Key init Finish!\n");
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/*********************************************************************
 * @fn      TKY_PeripheralInit
 *
 * @brief   ������������ʼ������
 *
 * @return  none
 */
static void TKY_PeripheralInit(void)
{
    /*You code here*/
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   ��ʱ��0�жϷ�����
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler( void )
{
    if( TMR0_GetITFlag( TMR0_3_IT_CYC_END ) )
    {
        TMR0_ClearITFlag( TMR0_3_IT_CYC_END );
        timerFlag=1;
    }
}
