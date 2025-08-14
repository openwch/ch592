/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_tmos.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/8/5
 * Description        : 触摸按键例程
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




static const uint8_t touch_wheel_ch[ TOUCH_WHEEL_ELEMENTS ] = {TOUCH_WHEEL_CHS};
uint16_t wheel_data[ TOUCH_WHEEL_ELEMENTS ] = {0};

static const uint8_t touch_slidel_ch[ TOUCH_SLIDER_ELEMENTS ] = {TOUCH_SLIDER_CHS};
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
    .p_elem_index = touch_slidel_ch,
    .threshold = 30,
    .decimal_point_percision = TOUCH_DECIMAL_POINT_PRECISION,
    .slider_resolution = TOUCH_SLIDER_RESOLUTION,
    .pdata = slider_data
    };

touch_cfg_t touch_cfg =
{
    .touch_button_cfg = NULL,
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
 * @fn      touch_dataProcess
 *
 * @brief   触摸数据处理函数（裸跑），打印获取到的按键触发情况
 *
 * @return  none
 */
void TKY_dataProcess(void)
{
    uint8_t key_val = 0x00;
    uint16_t Wheel_pros = 0;
    uint16_t Slider_pros = 0;
    static uint8_t touchinsflag = 0;
    static uint16_t print_time = 0;
    if(timerFlag)
    {
        timerFlag = 0;
        touch_Scan();
        if (!(touchinsflag & 0x0A))//当按键实例或者线性滑条实例在触发时，不触发当前滑轮实例
            {
                Wheel_pros = touch_GetWheelSliderData();
                if (Wheel_pros < TOUCH_OFF_VALUE)
                {
                    touchinsflag |= 1 << 2;
                    PRINT("Wheel_pros:%d\n",Wheel_pros);
                }
                else
                {
                    touchinsflag &= ~(1 << 2);
                }
            }

            if (!(touchinsflag & 0x06))//当按键实例或者滑轮实例在触发时，不触发当前线性滑条实例
            {
                Slider_pros = touch_GetLineSliderData();
                if (Slider_pros < TOUCH_OFF_VALUE)
                {
                    touchinsflag |= 1 << 3;
                    PRINT("Slider_pros:%d\n",Slider_pros);
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
 * @fn      touch_init
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
