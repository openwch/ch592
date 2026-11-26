/********************************** (C) COPYRIGHT *******************************
 * File Name          : app.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/8/5
 * Description        : 触摸滑轮滑条例程
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




static const uint8_t touch_wheel_ch[ TOUCH_WHEEL_ELEMENTS ] = {TOUCH_WHEEL_QUEUES};
uint16_t wheel_data[ TOUCH_WHEEL_ELEMENTS ] = {0};

static const uint8_t touch_slider_ch[ TOUCH_SLIDER_ELEMENTS ] = {TOUCH_SLIDER_QUEUES};
uint16_t slider_data[ TOUCH_SLIDER_ELEMENTS ] = {0};

touch_wheel_cfg_t p_wheel = {
    .num_elements = TOUCH_WHEEL_ELEMENTS,
    .p_elem_index = touch_wheel_ch,
    .threshold = 30,
    .decimal_point_percision = TOUCH_DECIMAL_POINT_PRECISION,
    .wheel_resolution = TOUCH_WHEEL_RESOLUTION,
    .pdata = wheel_data};

touch_slider_cfg_t p_slider = {
    .num_elements = TOUCH_SLIDER_ELEMENTS,
    .p_elem_index = touch_slider_ch,
    .threshold = 30,
    .decimal_point_percision = TOUCH_DECIMAL_POINT_PRECISION,
    .slider_resolution = TOUCH_SLIDER_RESOLUTION,
    .pdata = slider_data
    };

/* Wheel/Slider demo has no button instance, but the touch library
 * touch_InitVar() de-references touch_button_cfg without a NULL check.
 * Provide a valid empty (0-element) button config instead of NULL so
 * initialization does not crash; button detection stays inactive.
 * num_elements == 0, so p_elem_index / p_stbtn are never dereferenced. */
static touch_button_cfg_t p_button =
{
    .p_elem_index = NULL,
    .p_stbtn = NULL,
    .num_elements = 0
};

touch_cfg_t touch_cfg =
{
    .touch_button_cfg = &p_button,
    .touch_slider_cfg = &p_slider,
    .touch_wheel_cfg = &p_wheel
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
 * @brief   触摸数据处理函数（裸跑），打印获取到的按键触发情况
 *
 * @return  none
 */
void TKY_dataProcess(void)
{
    uint8_t key_val = 0x00;
    uint16_t wheel_pos = 0;
    uint16_t slider_pos = 0;
    static uint8_t touchinsflag = 0;
    static uint16_t print_time = 0;
    if(timerFlag)
    {
        timerFlag = 0;
        touch_Scan();
        if (!(touchinsflag & 0x0A))//当按键实例或者线性滑条实例在触发时，不触发当前滑轮实例
            {
                wheel_pos = touch_GetWheelSliderData();
                if (wheel_pos < TOUCH_OFF_VALUE)
                {
                    touchinsflag |= 1 << 2;
                    PRINT("wheel_pos:%d\n",wheel_pos);
                }
                else
                {
                    touchinsflag &= ~(1 << 2);
                }
            }

            if (!(touchinsflag & 0x06))//当按键实例或者滑轮实例在触发时，不触发当前线性滑条实例
            {
                slider_pos = touch_GetLineSliderData();
                if (slider_pos < TOUCH_OFF_VALUE)
                {
                    touchinsflag |= 1 << 3;
                    PRINT("slider_pos:%d\n",slider_pos);
                }
                else
                {
                    touchinsflag &= ~(1 << 3);
                }
            }

#if PRINT_EN
        print_time++;
        if(print_time == 500)
        {
            print_time = 0;
            touch_InfoDebug();
        }
#endif
    }

}


/*********************************************************************
 * @fn      TKY_Init
 *
 * @brief   触摸初始化函数（不使用tmos，需要设备开启定时器）
 *
 * @return  none
 */
void TKY_Init(void)
{
	TKY_PeripheralInit();       /* 初始化外设，例如背光和蜂鸣器等 */

    touch_Init(&touch_cfg);             /* 初始化触摸库  */

    TKY_SetSleepStatusValue( ~tkyPinAll.tkyQueueAll );

    TMR0_TimerInit(FREQ_SYS/1000);               //定时周期为1ms
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
 * @brief   触摸相关外设初始化函数
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
 * @brief   定时器0中断服务函数
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
