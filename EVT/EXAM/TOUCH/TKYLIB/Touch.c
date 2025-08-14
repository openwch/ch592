/********************************** (C) COPYRIGHT *******************************
 * File Name          : Touch.C
 * Author             : WCH
 * Version            : V1.6
 * Date               : 2021/12/1
 * Description        : 触摸按键例程
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "Touch.h"

/*********************
 *      DEFINES
 *********************/
#define WAKEUPTIME  50     //Sleep Time = 250 * SLEEP_TRIGGER_TIME(100ms) = 25s

/**********************
 *      VARIABLES
 **********************/
__attribute__ ((aligned (4))) uint32_t TKY_MEMBUF[ (TKY_MEMHEAP_SIZE - 1) / 4 + 1 ] = {0};
uint16_t keyData = 0, scanData = 0;
static uint16_t WheelData = TOUCH_OFF_VALUE; //触摸滑轮转换结果
static uint16_t SilderData = TOUCH_OFF_VALUE; //触摸滑条转换结果

static touch_cfg_t *p_touch_cfg = NULL;

uint8_t wakeUpCount = 0, wakeupflag = 0;


volatile TOUCH_S tkyPinAll = {0};
uint16_t tkyQueueAll = 0;
static const TKY_ChannelInitTypeDef my_tky_ch_init[TKY_QUEUE_END] = {TKY_CHS_INIT};

static const uint32_t TKY_Pin[14][2] = {
  {0x00, 0x00000010},//PA4
  {0x00, 0x00000020},//PA5
  {0x00, 0x00001000},//PA12
  {0x00, 0x00002000},//PA13
  {0x00, 0x00004000},//PA14
  {0x00, 0x00008000},//PA15
  {0x00, 0x00000000},//AIN6不存在，在此占位
  {0x00, 0x00000000},//AIN7不存在，在此占位

  {0x20, 0x00000001},//PB0,AIN8仅592X型号有
  {0x20, 0x00000040},//PB6,AIN9仅592X型号有

  {0x00, 0x00000040},//PA6,AIN10仅592X型号有
  {0x00, 0x00000080},//PA7,AIN11仅592X型号有
  {0x00, 0x00000100},//PA8,AIN12
  {0x00, 0x00000200} //PA9,AIN13
};
/**********************
 *  STATIC PROTOTYPES
 **********************/
static KEY_FIFO_T s_tKey;       /* 按键FIFO变量,结构体 */
static void touch_InitKeyHard(void);
static void touch_InitVar(touch_cfg_t *p);
static void touch_DetectKey(touch_button_cfg_t * p);
static void touch_Regcfg (void);
static void touch_Baseinit(void);
static void touch_Channelinit(void);
static uint16_t touch_DetecLineSlider(touch_slider_cfg_t * p_slider);
static uint16_t touch_DetectWheelSlider (touch_wheel_cfg_t * p_wheel);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/********************************************************************************************************
 * @fn      touch_Init
 * 
 * @brief   初始化按键. 该函数被 TKY_Init() 调用。
 *
 * @return  none
 */
void touch_Init(touch_cfg_t *p)
{
    touch_InitKeyHard();          /* 初始化按键硬件 */
    touch_InitVar(p);               /* 初始化按键变量 */
}

/********************************************************************************************************
 * @fn      touch_PutKey
 * @brief   将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
 * @param   _KeyCode - 按键代码
 * @return  none
 */
void touch_PutKey(uint8_t _KeyCode)
{
    s_tKey.Buf[s_tKey.Write] = _KeyCode;

    if (++s_tKey.Write  >= KEY_FIFO_SIZE)
    {
        s_tKey.Write = 0;
    }
}

/********************************************************************************************************
 * @fn      touch_GetKey
 * @brief   从按键FIFO缓冲区读取一个键值。
 * @param   无
 * @return  按键代码
 */
uint8_t touch_GetKey(void)
{
    uint8_t ret;

    if (s_tKey.Read == s_tKey.Write)
    {
        return KEY_NONE;
    }
    else
    {
        ret = s_tKey.Buf[s_tKey.Read];

        if (++s_tKey.Read >= KEY_FIFO_SIZE)
        {
            s_tKey.Read = 0;
        }
        return ret;
    }
}

/********************************************************************************************************
 * @fn      touch_GetKeyState
 * @brief   读取按键的状态
 * @param   _ucKeyID - 按键ID，从0开始
 * @return  1 - 按下
 *          0 - 未按下
*********************************************************************************************************
*/
uint8_t touch_GetKeyState(KEY_ID_E _ucKeyID)
{
    return p_touch_cfg->touch_button_cfg->p_stbtn[_ucKeyID].State;
}

/********************************************************************************************************
 * @fn      touch_SetKeyParam
 * @brief   设置按键参数
 * @param   _ucKeyID     - 按键ID，从0开始
 *          _LongTime    - 长按事件时间
 *          _RepeatSpeed - 连发速度
 * @return  none
 */
void touch_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed)
{
    p_touch_cfg->touch_button_cfg->p_stbtn[_ucKeyID].LongTime = _LongTime;          /* 长按时间 0 表示不检测长按键事件 */
    p_touch_cfg->touch_button_cfg->p_stbtn[_ucKeyID].RepeatSpeed = _RepeatSpeed;            /* 按键连发的速度，0表示不支持连发 */
    p_touch_cfg->touch_button_cfg->p_stbtn[_ucKeyID].RepeatCount = 0;                       /* 连发计数器 */
}


/********************************************************************************************************
 * @fn      touch_ClearKey
 * @brief   清空按键FIFO缓冲区
 * @param   无
 * @return  按键代码
 */
void touch_ClearKey(void)
{
    s_tKey.Read = s_tKey.Write;
}

/********************************************************************************************************
 * @fn      touch_ScanWakeUp
 * @brief   触摸扫描唤醒函数
 * @param   无
 * @return  无
 */
void touch_ScanWakeUp(void)
{
    wakeUpCount = WAKEUPTIME; //---唤醒时间---
    wakeupflag = 1;           //置成唤醒状态

    TKY_SetSleepStatusValue( 0x0000 ); //---设置0~11通道为非休眠状态,为接下来数秒时间内连续扫描做准备---
    dg_log("wake up for a while\n");
    touch_GPIOSleep();
}

/********************************************************************************************************
 * @fn      touch_ScanEnterSleep
 * @brief   触摸扫描休眠函数
 * @param   无
 * @return  无
 */
void touch_ScanEnterSleep(void)
{
    touch_GPIOSleep();
    wakeupflag = 0;       //置成睡眠状态:0,唤醒态:1
    TKY_SetSleepStatusValue( tkyPinAll.tkyQueueAll );
    dg_log("Ready to sleep\n");
}

/********************************************************************************************************
 * @fn      touch_Scan
 * @brief   扫描所有按键。非阻塞，被systick中断周期性的调用
 * @param   无
 * @return  无
 */
void touch_Scan(void)
{
    uint8_t i;

    TKY_LoadAndRun( );                     //---载入休眠前保存的部分设置---
    keyData = TKY_PollForFilter( );
    TKY_SaveAndStop();    //---对相关寄存器进行保存---

#if TKY_SLEEP_EN
    if (keyData)
    {
        wakeUpCount = WAKEUPTIME; //---唤醒时间---
    }
#endif

    touch_DetectKey(p_touch_cfg->touch_button_cfg);

    WheelData = touch_DetectWheelSlider(p_touch_cfg->touch_wheel_cfg);

    SilderData = touch_DetecLineSlider(p_touch_cfg->touch_slider_cfg);

}

/********************************************************************************************************
 * @fn      touch_GPIOModeCfg
 * @brief   触摸按键模式配置
 * @param   无
 * @return  无
 */
void touch_GPIOModeCfg(GPIOModeTypeDef mode)
{
    uint32_t pina = tkyPinAll.PaBit;
    uint32_t pinb = tkyPinAll.PbBit;
    switch(mode)
    {
        case GPIO_ModeIN_Floating:
        	R32_PA_PD_DRV &= ~pina;
        	R32_PA_PU &= ~pina;
        	R32_PA_DIR &= ~pina;
        	R32_PB_PD_DRV &= ~pinb;
        	R32_PB_PU &= ~pinb;
        	R32_PB_DIR &= ~pinb;
            break;

        case GPIO_ModeOut_PP_5mA:
            R32_PA_PU &= ~pina;
            R32_PA_PD_DRV &= ~pina;
            R32_PA_DIR |= pina;
            R32_PB_PU &= ~pinb;
            R32_PB_PD_DRV &= ~pinb;
            R32_PB_DIR |= pinb;
            break;
        default:
            break;
    }
}

void touch_IOSetAdcState(uint8_t ch)
{
	(*((PUINT32V)0x400010B4+TKY_Pin[ch][0])) &= ~TKY_Pin[ch][1];
	(*((PUINT32V)0x400010B0+TKY_Pin[ch][0])) &= ~TKY_Pin[ch][1];
	(*((PUINT32V)0x400010A0+TKY_Pin[ch][0])) &= ~TKY_Pin[ch][1];
}

void touch_SingleChDischarge(uint8_t ch){
	(*((PUINT32V)(0x400010A0+TKY_Pin[ch][0]))) |= TKY_Pin[ch][1];
	(*((PUINT32V)(0x400010AC+TKY_Pin[ch][0]))) = TKY_Pin[ch][1];
}

/********************************************************************************************************
 * @fn      touch_GPIOSleep
 * @brief   配置触摸按键为休眠状态
 * @param   无
 * @return  无
 */
void touch_GPIOSleep(void)
{
    uint32_t pina = tkyPinAll.PaBit;
    uint32_t pinb = tkyPinAll.PbBit;
    R32_PA_PU &= ~pina;
    R32_PA_PD_DRV &= ~pina;
    R32_PA_DIR |= pina;
    R32_PA_CLR |= pina;

    R32_PB_PU &= ~pinb;
    R32_PB_PD_DRV &= ~pinb;
    R32_PB_DIR |= pinb;
    R32_PB_CLR |= pinb;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/********************************************************************************************************
 * @fn      touch_InitKeyHard
 * @brief   初始化触摸按键
 * @param   无
 * @return  无
 */
static void touch_InitKeyHard(void)
{
    touch_Regcfg();
    touch_Baseinit( );
    touch_Channelinit( );
}


/********************************************************************************************************
 * @fn      touch_InitVar
 * @brief   初始化触摸按键变量
 * @param   无
 * @return  无
 */
static void touch_InitVar(touch_cfg_t *p)
{
    uint8_t i;

    p_touch_cfg = p;

    /* 对按键FIFO读写指针清零 */
    s_tKey.Read = 0;
    s_tKey.Write = 0;

    /* 给每个按键结构体成员变量赋一组缺省值 */
    for (i = 0; i < p_touch_cfg->touch_button_cfg->num_elements; i++)
    {
        p_touch_cfg->touch_button_cfg->p_stbtn[i].LongTime = KEY_LONG_TIME;             /* 长按时间 0 表示不检测长按键事件 */
        p_touch_cfg->touch_button_cfg->p_stbtn[i].Count = KEY_FILTER_TIME / 2;          /* 计数器设置为滤波时间的一半 */
        p_touch_cfg->touch_button_cfg->p_stbtn[i].State = 0;                            /* 按键缺省状态，0为未按下 */
        p_touch_cfg->touch_button_cfg->p_stbtn[i].RepeatSpeed = KEY_REPEAT_TIME;                      /* 按键连发的速度，0表示不支持连发 */
        p_touch_cfg->touch_button_cfg->p_stbtn[i].RepeatCount = 0;                      /* 连发计数器 */
    }

    /* 如果需要单独更改某个按键的参数，可以在此单独重新赋值 */
    /* 比如，我们希望按键1按下超过1秒后，自动重发相同键值 */
//    s_tBtn[KID_K1].LongTime = 100;
//    s_tBtn[KID_K1].RepeatSpeed = 5; /* 每隔50ms自动发送键值 */

}



/********************************************************************************************************
 * @fn      touch_InfoDebug
 * @brief   触摸数据打印函数
 * @param   无
 * @return  无
 */
 
void touch_InfoDebug(void)
{
    uint8_t i;
    int16_t data_dispNum[ TKY_MAX_QUEUE_NUM ]={0};
	int16_t bl,vl;

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
#if TKY_FILTER_MODE == FILTER_MODE_1
        bl = TKY_GetCurQueueBaseLine( i );
        vl = TKY_GetCurQueueValue( i );
        if(bl>vl)   data_dispNum[ i ] =  bl-vl ;
        else        data_dispNum[ i ] =  vl-bl ;
#else
        data_dispNum[ i ] = TKY_GetCurQueueValue( i );
#endif
    }

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", data_dispNum[i]);
    } dg_log("\n");

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        data_dispNum[ i ] = TKY_GetCurQueueBaseLine( i );
    }

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", data_dispNum[i]);
    } dg_log("\n");
#if TKY_FILTER_MODE == FILTER_MODE_1
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", TKY_GetCurQueueValue( i ));
    }dg_log("\n");
#endif
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", TKY_GetCurQueueRealVal( i ));
    }dg_log("\r\n");
#if TKY_FILTER_MODE == FILTER_MODE_7
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	dg_log("%04d,", TKY_GetCurQueueValue2( i ));
    }dg_log("\r\n");
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	dg_log("%04d,", TKY_GetCurQueueBaseLine2( i ));
    }dg_log("\r\n");
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	dg_log("%04d,", TKY_GetCurQueueRealVal2( i ));
    }dg_log("\r\n");
#endif
    dg_log("\r\n");

}

/********************************************************************************************************
 * @fn      touch_DetectKey
 * @brief   检测一个按键。非阻塞状态，必须被周期性的调用。
 * @param   i - 按键结构变量指针
 * @return  无
 */
static void touch_DetectKey(touch_button_cfg_t * p)
{
    KEY_T *pBtn;

    if (p == NULL)
    {
        return ;
    }

    for (uint8_t i = 0; i < p->num_elements; i++)
    {
        /*按键按下*/
        pBtn = NULL;
        pBtn = &p_touch_cfg->touch_button_cfg->p_stbtn[ i ];
        if (keyData & (1 << p->p_elem_index[i] ))          // pBtn->IsKeyDownFunc()==1
        {
            if (pBtn->State == 0)
            {
                pBtn->State = 1;
#if !KEY_MODE
                /* 发送按钮按下的消息 */
                touch_PutKey((uint8_t)(3 * i + 1));
#endif
            }

            /*处理长按键*/
            if (pBtn->LongTime > 0)
            {
                if (pBtn->LongCount < pBtn->LongTime)
                {
                    /* 发送按钮长按下的消息 */
                    if (++pBtn->LongCount == pBtn->LongTime)
                    {
#if !KEY_MODE
                        pBtn->State = 2;

                        /* 键值放入按键FIFO */
                        touch_PutKey((uint8_t)(3 * i + 3));
#endif
                    }
                }
                else
                {
                    if (pBtn->RepeatSpeed > 0)
                    {
                        if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
                        {
                            pBtn->RepeatCount = 0;
#if !KEY_MODE
                            /* 长按键后，每隔pBtn->RepeatSpeed*10ms发送1个按键 */
                            touch_PutKey((uint8_t)(3 * i + 1));
#endif
                        }
                    }
                }
            }
    }
    else
    {
            if (pBtn->State)
            {
#if KEY_MODE
                if(pBtn->State == 1)
                /* 发送按钮按下的消息 */
                touch_PutKey((uint8_t)(3 * i + 1));
#endif
                pBtn->State = 0;

#if !KEY_MODE
                /* 松开按键KEY_FILTER_TIME后 发送按钮弹起的消息 */
                touch_PutKey((uint8_t)(3 * i + 2));
#endif
            }

            pBtn->LongCount = 0;
            pBtn->RepeatCount = 0;
        }
    }
}

static void touch_Regcfg (void)
{
    R8_ADC_CFG = RB_ADC_POWER_ON | RB_ADC_BUF_EN | (ADC_PGA_0 << 4) | (SampleFreq_8 << 6);
    R8_ADC_CONVERT &= ~(RB_ADC_PGA_GAIN2);// | RB_ADC_SAMPLE_TIME);
//    R8_ADC_CONVERT |= RB_ADC_SAMPLE_TIME;
    R8_TKEY_CFG = RB_TKEY_PWR_ON | RB_TKEY_CURRENT;

#if TKY_SHIELD_EN
    R8_TKEY_CFG |= RB_TKEY_DRV_EN;
#endif

    TKY_SaveCfgReg();
}

/********************************************************************************************************
 * @fn      touch_Baseinit
 * @brief   触摸基础库初始化
 * @param   无
 * @return  无
 */
static void touch_Baseinit(void)
{
    TKY_BaseInitTypeDef TKY_BaseInitStructure = {0};
    for(uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++)  //初始化tkyPinAll、tkyQueueAll变量
    {
    	if(TKY_Pin[my_tky_ch_init[i].channelNum][0] == 0x00)
    	{
    		tkyPinAll.PaBit |= TKY_Pin[my_tky_ch_init[i].channelNum][1];
    	}
    	else if(TKY_Pin[my_tky_ch_init[i].channelNum][0] == 0x20)
      {
    		tkyPinAll.PbBit |= TKY_Pin[my_tky_ch_init[i].channelNum][1];
    	}
    	tkyPinAll.tkyQueueAll |= 1<<i;
    }
    dg_log("tP : %08x,%08x; tQ : %04x\n",tkyPinAll.PaBit,tkyPinAll.PbBit,tkyPinAll.tkyQueueAll);
    R8_TKEY_CFG|=RB_TKEY_CURRENT;
#if (TKY_SHIELD_EN)
    tkyPinAll.PaBit |= TKY_SHIELD_PIN;
#endif

    touch_GPIOSleep();  //拉低所有触摸pin脚

#if (TKY_SHIELD_EN)
    tkyPinAll.PaBit &= ~TKY_SHIELD_PIN;
    GPIOA_ModeCfg(TKY_SHIELD_PIN, GPIO_ModeIN_Floating);//Shield Pin， only for CH58x series
#endif

    //----------触摸按键基础设置初始化--------
    TKY_BaseInitStructure.filterMode = TKY_FILTER_MODE;
    TKY_BaseInitStructure.shieldEn = TKY_SHIELD_EN;
    TKY_BaseInitStructure.singlePressMod = TKY_SINGLE_PRESS_MODE;
    TKY_BaseInitStructure.filterGrade = TKY_FILTER_GRADE;
    TKY_BaseInitStructure.maxQueueNum = TKY_MAX_QUEUE_NUM;
    TKY_BaseInitStructure.baseRefreshOnPress = TKY_BASE_REFRESH_ON_PRESS;
    //---基线更新速度，baseRefreshSampleNum和filterGrade，与基线更新速度成反比，基线更新速度还与代码结构相关，可通过函数GetCurQueueBaseLine来观察---
    TKY_BaseInitStructure.baseRefreshSampleNum = TKY_BASE_REFRESH_SAMPLE_NUM;
    TKY_BaseInitStructure.baseUpRefreshDouble = TKY_BASE_UP_REFRESH_DOUBLE;
    TKY_BaseInitStructure.baseDownRefreshSlow = TKY_BASE_DOWN_REFRESH_SLOW;
    TKY_BaseInitStructure.tkyBufP = TKY_MEMBUF;
    TKY_BaseInit( TKY_BaseInitStructure );
}

/********************************************************************************************************
 * @fn      touch_Channelinit
 * @brief   触摸通道初始化
 * @param   无
 * @return  无
 */
static void touch_Channelinit(void)
{
    uint8_t error_flag = 0;
    uint16_t chx_mean = 0;

    for(uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	TKY_CHInit(my_tky_ch_init[i]);
    }

    for(uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {

    	chx_mean = TKY_GetCurChannelMean(my_tky_ch_init[i].channelNum, my_tky_ch_init[i].chargeTime,
										 my_tky_ch_init[i].disChargeTime, 1000);

    	if(chx_mean < 3400 || chx_mean > 3800)
    	{
    		error_flag = 1;
    	}
    	else
    	{
    		TKY_SetCurQueueBaseLine(i, chx_mean);
    	}
    	dg_log("queue : %d ch : %d , mean : %d\n",i,my_tky_ch_init[i].channelNum,chx_mean);

    }
    //充放电基线值异常，重新校准基线值
    if(error_flag != 0)
    {
    	touch_GPIOSleep();  //拉低所有触摸pin脚
        dg_log("\n\nCharging parameters error, preparing for recalibration ...\n\n");
        uint8_t charge_time;
        for (uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++) {       //按最大序列数进行ADC通道转换
          charge_time = 0,chx_mean = 0;
          touch_IOSetAdcState(my_tky_ch_init[i].channelNum);
          while (1)
          {
              chx_mean = TKY_GetCurChannelMean(my_tky_ch_init[i].channelNum, charge_time,3, 1000);

//              dg_log("testing .... chg : %d, baseline : %d\n",charge_time,chx_mean);//打印基线值

              if ((charge_time == 0) && ((chx_mean > 3800))) {//低于最小充电参数
                  dg_log("Error, %u KEY%u Too small Cap,Please check the hardware !\r\n",chx_mean,i);
                  break;
              }
              else {
                  if ((chx_mean > 3200) &&(chx_mean < 3800)) {//充电参数正常
                      TKY_SetCurQueueBaseLine(i, chx_mean);
                      TKY_SetCurQueueChargeTime(i,charge_time,3);
                      dg_log("channel:%u, chargetime:%u,BaseLine:%u\r\n",
                            i, charge_time, chx_mean);
                      break;
                  }else if(chx_mean >= 3800)
                  {
                	  TKY_SetCurQueueBaseLine(i, TKY_GetCurChannelMean(my_tky_ch_init[i].channelNum, charge_time-1,3, 20));
                	  TKY_SetCurQueueChargeTime(i,charge_time-1,3);
                	  dg_log("Warning,channel:%u Too large Current, chargetime:%u,BaseLine:%u\r\n",
                	                              i, charge_time, chx_mean);
                	  break;
                  }
                  charge_time++;
                  if (charge_time > 0x1f) {    //超出最大充电参数
                      dg_log("Error, Chargetime Max,KEY%u Too large Cap,Please check the hardware !\r\n",i);
                      break;
                  }
              }
          }
          touch_SingleChDischarge(my_tky_ch_init[i].channelNum);
//          GPIOA_ModeCfg(TKY_Pin[my_tky_ch_init[i].channelNum],GPIO_ModeIN_Floating);
        }
    }
    TKY_SaveAndStop();
}
/********************************************************************************************************
 * @fn      touch_DetectWheelSlider
 * @brief   触摸滑轮数据处理
 * @param   无
 * @return  无
 */
static  uint16_t touch_DetectWheelSlider (touch_wheel_cfg_t * p_wheel)
{
    uint8_t loop;
    uint8_t max_data_idx;
    uint16_t d1;
    uint16_t d2;
    uint16_t d3;
    uint16_t wheel_rpos;
    uint16_t dsum;
    uint16_t unit;
    uint8_t num_elements;
    uint16_t p_threshold;
    uint16_t * wheel_data;

    if (p_wheel == NULL)
    {
        return TOUCH_OFF_VALUE;
    }

    num_elements = p_wheel->num_elements;
    p_threshold = p_wheel->threshold;
    wheel_data = p_wheel->pdata;

    if (num_elements < 3)
    {
        return TOUCH_OFF_VALUE;
    }

    for (loop = 0; loop < p_wheel->num_elements; loop++)
    {
        wheel_data[ loop ] = TKY_GetCurQueueValue (p_wheel->p_elem_index[ loop ]);
    }

    /* Search max data in slider */
    max_data_idx = 0;
    for (loop = 0; loop < (num_elements - 1); loop++)
    {
        if (wheel_data[ max_data_idx ] < wheel_data[ loop + 1 ])
        {
            max_data_idx = (uint8_t) (loop + 1);
        }
    }
    /* Array making for wheel operation          */
    /*    Maximum change CH_No -----> Array"0"    */
    /*    Maximum change CH_No + 1 -> Array"2"    */
    /*    Maximum change CH_No - 1 -> Array"1"    */
    if (0 == max_data_idx)
    {
        d1 = (uint16_t) (wheel_data[ 0 ] - wheel_data[ num_elements - 1 ]);
        d2 = (uint16_t) (wheel_data[ 0 ] - wheel_data[ 1 ]);
        dsum = (uint16_t) (wheel_data[ 0 ] + wheel_data[ 1 ] + wheel_data[ num_elements - 1 ]);
    }
    else if ((num_elements - 1) == max_data_idx)
    {
        d1 = (uint16_t) (wheel_data[ num_elements - 1 ] - wheel_data[ num_elements - 2 ]);
        d2 = (uint16_t) (wheel_data[ num_elements - 1 ] - wheel_data[ 0 ]);
        dsum = (uint16_t) (wheel_data[ 0 ] + wheel_data[ num_elements - 2 ] + wheel_data[ num_elements - 1 ]);
    }
    else
    {
        d1 = (uint16_t) (wheel_data[ max_data_idx ] - wheel_data[ max_data_idx - 1 ]);
        d2 = (uint16_t) (wheel_data[ max_data_idx ] - wheel_data[ max_data_idx + 1 ]);
        dsum = (uint16_t) (wheel_data[ max_data_idx + 1 ] + wheel_data[ max_data_idx ] + wheel_data[ max_data_idx - 1 ]);
    }

    if (0 == d1)
    {
        d1 = 1;
    }
    /* Constant decision for operation of angle of wheel */
    if (dsum > p_threshold)
    {
        d3 = (uint16_t) (p_wheel->decimal_point_percision + ((d2 * p_wheel->decimal_point_percision) / d1));

        unit = (uint16_t) (p_wheel->wheel_resolution / num_elements);
        wheel_rpos = (uint16_t) (((unit * p_wheel->decimal_point_percision) / d3) + (unit * max_data_idx));

        /* Angle division output */
        /* diff_angle_ch = 0 -> 359 ------ diff_angle_ch output 1 to 360 */
        if (0 == wheel_rpos)
        {
            wheel_rpos = p_wheel->wheel_resolution ;
        }
        else if ((p_wheel->wheel_resolution + 1) < wheel_rpos)
        {
            wheel_rpos = 1;
        }
        else
        {
            /* Do Nothing */
        }
    }
    else
    {
        wheel_rpos = TOUCH_OFF_VALUE;
    }

    return wheel_rpos;
}

/********************************************************************************************************
 * @fn      touch_DetectWheelSlider
 * @brief   触摸滑条数据处理
 * @param   无
 * @return  滑条坐标
 */
static uint16_t touch_DetecLineSlider(touch_slider_cfg_t * p_slider)
{

    uint8_t loop;
    uint8_t max_data_idx;
    uint16_t d1;
    uint16_t d2;
    uint16_t d3;
    uint16_t slider_rpos;
    uint16_t resol_plus;
    uint16_t dsum;
    uint8_t num_elements = 0;
    uint16_t p_threshold = 0;
    uint16_t * slider_data = 0;

    if (p_slider == NULL)
    {
        return TOUCH_OFF_VALUE;
    }

    num_elements = p_slider->num_elements;
    p_threshold = p_slider->threshold;
    slider_data = p_slider->pdata;

    if (num_elements < 3)
    {
        return TOUCH_OFF_VALUE;
    }

    for (uint8_t loop = 0; loop < num_elements; loop++)
    {
        slider_data[ loop ] = TKY_GetCurQueueValue (p_slider->p_elem_index[ loop ]);
    }
    /* Search max data in slider */
    max_data_idx = 0;
    for (loop = 0; loop < (num_elements - 1); loop++)
    {
        if (slider_data[max_data_idx] < slider_data[loop + 1])
        {
            max_data_idx = (uint8_t)(loop + 1);
        }
    }

    /* Array making for slider operation-------------*/
    /*     |    Maximum change CH_No -----> Array"0"    */
    /*     |    Maximum change CH_No + 1 -> Array"2"    */
    /*     |    Maximum change CH_No - 1 -> Array"1"    */
#if 0
    if (0 == max_data_idx)
    {
        d1 = (uint16_t)(slider_data[0] - slider_data[2]);
        d2 = (uint16_t)(slider_data[0] - slider_data[1]);
    }
    else if ((num_elements - 1) == max_data_idx)
    {
        d1 = (uint16_t)(slider_data[num_elements - 1] - slider_data[num_elements - 2]);
        d2 = (uint16_t)(slider_data[num_elements - 1] - slider_data[num_elements - 3]);
    }
    else
    {
        d1 = (uint16_t)(slider_data[max_data_idx] - slider_data[max_data_idx - 1]);
        d2 = (uint16_t)(slider_data[max_data_idx] - slider_data[max_data_idx + 1]);
    }

    dsum = (uint16_t)(d1 + d2);

    /* Constant decision for operation of angle of slider */
    /* Scale results to be 0-TOUCH_SLIDER_RESOLUTION */
    if (dsum > p_threshold)
    {
        if (0 == d1)
        {
            d1 = 1;
        }

        /* x : y = d1 : d2 */
        d3 = (uint16_t)(p_slider->decimal_point_percision + ((d2 * p_slider->decimal_point_percision) / d1));

        slider_rpos = (uint16_t)(((p_slider->decimal_point_percision * p_slider->slider_resolution) / d3) + (p_slider->slider_resolution * max_data_idx));

        resol_plus = (uint16_t)(p_slider->slider_resolution * (num_elements - 1));

        if (0 == slider_rpos)
        {
            slider_rpos = 1;
        }
        else if (slider_rpos >= resol_plus)
        {
            slider_rpos = (uint16_t)(((slider_rpos - resol_plus) * 2) + resol_plus);
            if (slider_rpos > (p_slider->slider_resolution * num_elements))
            {
                slider_rpos = p_slider->slider_resolution;
            }
            else
            {
                slider_rpos = (uint16_t)(slider_rpos / num_elements);
            }
        }
        else if (slider_rpos <= p_slider->slider_resolution)
        {
            if (slider_rpos < (p_slider->slider_resolution / 2))
            {
                slider_rpos = 1;
            }
            else
            {
                slider_rpos = (uint16_t)(slider_rpos - (p_slider->slider_resolution / 2));
                if (0 == slider_rpos)
                {
                    slider_rpos = 1;
                }
                else
                {
                    slider_rpos = (uint16_t)((slider_rpos * 2) / num_elements);
                }
            }
        }
        else
        {
            slider_rpos = (uint16_t)(slider_rpos / num_elements);
        }
    }
    else
    {
        slider_rpos = TOUCH_OFF_VALUE;
    }

    #else
    // int16_t dval;
    uint16_t unit;

    if (0 == max_data_idx)
    {
        d1 = (uint16_t) (slider_data[ 0 ] - slider_data[ num_elements - 1 ]);
        d2 = (uint16_t) (slider_data[ 0 ] - slider_data[ 1 ]);
        dsum = (uint16_t) (slider_data[ 0 ] + slider_data[ 1 ] + slider_data[ num_elements - 1 ]);
    }
    else if ((num_elements - 1) == max_data_idx)
    {
        d1 = (uint16_t) (slider_data[ num_elements - 1 ] - slider_data[ num_elements - 2 ]);
        d2 = (uint16_t) (slider_data[ num_elements - 1 ] - slider_data[ 0 ]);
        dsum = (uint16_t) (slider_data[ 0 ] + slider_data[ num_elements - 2 ] + slider_data[ num_elements - 1 ]);
    }
    else
    {
        d1 = (uint16_t) (slider_data[ max_data_idx ] - slider_data[ max_data_idx - 1 ]);
        d2 = (uint16_t) (slider_data[ max_data_idx ] - slider_data[ max_data_idx + 1 ]);
        dsum = (uint16_t) (slider_data[ max_data_idx + 1 ] + slider_data[ max_data_idx ] + slider_data[ max_data_idx - 1 ]);
    }

    if (0 == d1)
    {
        d1 = 1;
    }
    /* Constant decision for operation of angle of wheel    */
    if (dsum > p_threshold)
    {
        d3 = (uint16_t) (p_slider->decimal_point_percision + ((d2 * p_slider->decimal_point_percision) / d1));

        unit = (uint16_t) (p_slider->slider_resolution / num_elements);
        slider_rpos = (uint16_t) (((unit * p_slider->decimal_point_percision) / d3) + (unit * max_data_idx));

        /* Angle division output */
        /* diff_angle_ch = 0 -> 359 ------ diff_angle_ch output 1 to 360 */
        if (0 == slider_rpos)
        {
            slider_rpos = p_slider->slider_resolution;
        }
        else if ((p_slider->slider_resolution + 1) < slider_rpos)
        {
            slider_rpos = 1;
        }
        else
        {
            /* Do Nothing */
        }
    }
    else
    {
        slider_rpos = TOUCH_OFF_VALUE;
    }
#endif
    return slider_rpos;
}

uint16_t touch_GetLineSliderData(void)
{
    return SilderData;
}

uint16_t touch_GetWheelSliderData(void)
{
    return WheelData;
}
