#ifndef __WCH_TOUCH_H__
#define __WCH_TOUCH_H__
//------------------------------------------

//---------------filter-----------------
#define FILTER_MODE_3             3       //--滤波器模式3, 可输出多个按键--
#define FILTER_MODE_CS10          2       //--CS10V专用滤波器模式--
//---------------single key mode-----------------
#define TKY_SINGLE_KEY_MULTI      0       //--多按键输出，即超过阈值的按键都会触发
#define TKY_SINGLE_KEY_MAX        1       //--最大值单按键输出，即超过阈值的按键中只上报变化量最大的按键
#define TKY_SINGLE_KEY_MUTU       2       //--互斥单按键输出，即当前按键释放后才会上报下一个变化量最大的按键，
                                          //  否则其他按键无论变化量多大都不上报

//--------------lib param-----------------
#define TKY_BUFLEN                28
//---------------------------------------
typedef struct
{
    uint8_t maxQueueNum;                    //--测试队列数量--
    uint8_t singlePressMod;                 //--单按键模式---
    uint8_t shieldEn;                       //--屏蔽使能---
    uint8_t filterMode;                     //--滤波器模式--
    uint8_t filterGrade;                    //--滤波器等级--
    uint8_t peakQueueNum;                   //--按键最大偏移队列---
    uint8_t peakQueueOffset;                //--按键最大偏移队列的偏移值---
    uint8_t baseRefreshOnPress;             //--基线在按键按下时是否进行--
    uint8_t baseUpRefreshDouble;            //--基线向上刷新倍速参数---
    uint8_t baseDownRefreshSlow;            //--基线向下更新降速参数---
    uint8_t rfu[2];
    uint32_t baseRefreshSampleNum;          //--基线刷新采样次数--
    uint32_t *tkyBufP;                      //--测试通道数据缓冲区指针--
}TKY_BaseInitTypeDef;

typedef struct
{
    uint8_t queueNum;                       //--该通道在测试队列的序号--
    uint8_t channelNum;                     //--该通道对应的ADC通道标号--
    uint16_t chargeTime;                    //--该通道充电时间--
    uint16_t disChargeTime;                 //--该通道放电时间--
    uint16_t baseLine;                      //--基线--
    uint16_t threshold;                     //--阈值--
    uint16_t threshold2;                    //--阈值2--
    uint8_t sleepStatus;                    //--休眠--
}TKY_ChannelInitTypeDef;


/*******************************************************************************
 * Function Name  : TKY_BaseInit
 * Description    : TouchKey总体参数初始化
 * Input          : TKY_BaseInitStruct，初识化的参数
 * Return         : reData, 0为初识化成功，其他为失败故障。
 *******************************************************************************/
extern uint8_t TKY_BaseInit(TKY_BaseInitTypeDef TKY_BaseInitStruct);

/*******************************************************************************
 * Function Name  : TKY_CHInit
 * Description    : TouchKey通道参数初始化
 * Input          : TKY_CHInitStruct，初始化的参数
 * Return         : reData, 0为初识化成功，其他为失败故障。
 *******************************************************************************/
extern uint8_t TKY_CHInit(TKY_ChannelInitTypeDef TKY_CHInitStruct);

/*******************************************************************************
 * Function Name  : TKY_GetCurChannelMean
 * Description    : 获取当前通道的平均值，主要用于设置baseline和门槛值用途。
 * Input          : curChNum当前转换通道，chargeTime是当前通道的充放电时间，
                    averageNum是平均求和总数。
 * Return         : 当前测试的ADC值。
 *******************************************************************************/
extern uint16_t TKY_GetCurChannelMean(uint8_t curChNum, uint16_t chargeTime, uint16_t disChargeTime, uint16_t averageNum);

/*******************************************************************************
 * Function Name  : TKY_GetCurQueueValue
 * Description    : 获取指定通道处理后的值。
 * Input          : curQueueNum当前需要取值的通道。
 * Return         : 当前通道的处理值。
 *******************************************************************************/
extern uint16_t TKY_GetCurQueueValue(uint8_t curQueueNum);

/*******************************************************************************
 * Function Name  : TKY_PollForFilerMode_3
 * Description    : TouchKey主循环轮询模式，适合滤波器3滤波，执行过程阻塞
 * Input          : 无
 * Return         : pressData, 0为未按下，1为按下。
 *******************************************************************************/
extern uint16_t TKY_PollForFilterMode_3(void);

/*******************************************************************************
 * Function Name  : TKY_PollForFilterMode_CS10
 * Description    : TouchKey主循环轮询模式，适合用于CS10V测试
 * Input          : 无
 * Return         : pressData, 0为未按下，1为按下。
 *******************************************************************************/
extern uint16_t TKY_PollForFilterMode_CS10(void);

/*******************************************************************************
 * Function Name  : TKY_ScanForWakeUp
 * Description    : TouchKey休眠检测，主循环定时轮询
 * Input          : 无
 * Return         : pressData, 0为未按下，1为按下。
 *******************************************************************************/
extern uint16_t TKY_ScanForWakeUp(uint16_t scanBitValue);

/*******************************************************************************
 * Function Name  : TKY_SetCurQueueSleepStatus
 * Description    : 设置指定通道的休眠状态。
 * Input          : curQueueNum当前需要设置的通道，sleepStatus为0时不休眠，非0休眠。
 * Return         : 返回0则设置成功，1为超出队列最大长度。
 *******************************************************************************/
extern uint8_t TKY_SetCurQueueSleepStatus(uint8_t curQueueNum, uint8_t sleepStatus);

/*******************************************************************************
 * Function Name  : TKY_SetSleepStatusValue
 * Description    : 设置多个指定通道的休眠状态。
 * Input          : setValue以检测队列顺序按位设置睡眠状态，0：不休眠，1：休眠。
 * Return         : 返回0则设置成功，1为超出队列最大长度。
 *******************************************************************************/
extern uint8_t TKY_SetSleepStatusValue (uint16_t setValue);

/*******************************************************************************
 * Function Name  : TKY_GetSleepStatusValue
 * Description    : 设置多个指定通道的休眠状态。
 * Input          : setValue以检测队列顺序按位设置睡眠状态，0：不休眠，1：休眠。
 * Return         : 返回0则设置成功，1为超出队列最大长度。
 *******************************************************************************/
extern uint16_t TKY_GetSleepStatusValue(void);

/*******************************************************************************
 * Function Name  : TKY_SetCurQueueChargeTime
 * Description    : 设置指定通道的充电参数。
 * Input          : curQueueNum当前需要设置的通道。
 * Return         : 返回0则设置成功，1为超出队列最大长度。
 *******************************************************************************/
extern uint8_t TKY_SetCurQueueChargeTime (uint8_t curQueueNum,
                                          uint16_t chargeTime,
                                          uint16_t disChargeTime);
/*******************************************************************************
 * Function Name  : TKY_SetCurQueueThreshold
 * Description    : 设置指定通道的门槛值。
 * Input          : curQueueNum当前需要设置的通道。
 * Return         : 返回0则设置成功，1为超出队列最大长度。
 *******************************************************************************/
extern uint8_t TKY_SetCurQueueThreshold (uint8_t curQueueNum,
                                         uint16_t threshold,
                                         uint16_t threshold2);

/*******************************************************************************
 * Function Name  : TKY_GetCurIdleStatus
 * Description    : 获取空闲状态
 * Input          : 无
 * Return         : 返回是否空闲
 *******************************************************************************/
extern uint8_t TKY_GetCurIdleStatus(void);

/*******************************************************************************
 * Function Name  : TKY_GetCurVersion
 * Description    : 获取当前版本号。
 * Input          : 无
 * Return         : 获取当前版本号。
 *******************************************************************************/
extern uint16_t TKY_GetCurVersion(void);

/*******************************************************************************
 * Function Name  : TKY_GetCurQueueBaseLine
 * Description    : 获取指定通道基线值。
 * Input          : curQueueNum当前需要取值的通道。
 * Return         : 当前通道的处理值。
 *******************************************************************************/
extern uint16_t TKY_GetCurQueueBaseLine(uint8_t curQueueNum);

/*******************************************************************************
 * Function Name  : TKY_GetCurQueueRealVal
 * Description    : 获取指定通道原始测量值。
 * Input          : curQueueNum当前需要取值的通道。
 * Return         : 当前通道的原始测量值。
 *******************************************************************************/
extern uint16_t TKY_GetCurQueueRealVal(uint8_t curQueueNum);

/*******************************************************************************
 * Function Name  : TKY_SetCurQueueBaseLine
 * Description    : 设置指定通道基线值。
 * Input          : curQueueNum当前需要设置值的通道，baseLineValue当前通道的设置值。
 * Return         : 当前通道的处理值。
 *******************************************************************************/
extern void TKY_SetCurQueueBaseLine(uint8_t curQueueNum, uint16_t baseLineValue);

/*******************************************************************************
 * Function Name  : TKY_SetBaseRefreshSampleNum
 * Description    : 基线向上刷新倍速参数。
 * Input          : newValue新参数
 * Return         : 无
 *******************************************************************************/
extern void TKY_SetBaseRefreshSampleNum(uint32_t newValue);

/*******************************************************************************
 * Function Name  : TKY_SetBaseDownRefreshSlow
 * Description    : 基线向上刷新倍速参数。
 * Input          : newValue新参数
 * Return         : 无
 *******************************************************************************/
extern void TKY_SetBaseUpRefreshDouble(uint8_t newValue);

/*******************************************************************************
 * Function Name  : TKY_SetBaseDownRefreshSlow
 * Description    : 设置基线向下更新降速参数。
 * Input          : newValue新参数
 * Return         : 无
 *******************************************************************************/
extern void TKY_SetBaseDownRefreshSlow(uint8_t newValue);

/*******************************************************************************
 * Function Name  : TKY_SetFilterMode
 * Description    : 设置当前工作的滤波器模式
 * Input          : newValue：滤波器模式
 * Return         : 无
 *******************************************************************************/
extern void TKY_SetFilterMode(uint8_t newValue);

/*******************************************************************************
 * Function Name  : TKY_ClearHistoryData
 * Description    : 清除历史数据。
 * Input          : curFilterMode当前滤波模式
 * Return         : 无
 *******************************************************************************/
extern void TKY_ClearHistoryData(uint8_t curFilterMode);

/*******************************************************************************
 * Function Name  : TKY_SaveCfgReg
 * Description    : CH32系列芯片在外部校准好库的配置参数后进行保存
 * Input          : 无
 * Return         : 无
 *******************************************************************************/
extern void TKY_SaveCfgReg(void);

/*******************************************************************************
 * Function Name  : TKY_SaveAndStop
 * Description    : TouchKey的tky的cfg寄存器值保存，并停止运行
 * Input          : 无
 * Return         : 无
 *******************************************************************************/
extern void TKY_SaveAndStop(void);

/*******************************************************************************
 * Function Name  : TKY_LoadAndRun
 * Description    : TouchKey的tky的cfg寄存器值载入并恢复运行
 * Input          : 无
 * Return         : 无
 *******************************************************************************/
extern void TKY_LoadAndRun(void);

#endif
