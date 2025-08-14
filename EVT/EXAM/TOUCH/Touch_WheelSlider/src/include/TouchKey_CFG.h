/********************************** (C) COPYRIGHT *******************************
* File Name : TouchKey_CFG.h
* Author             : WCH
* Version            : V1.0
* Date               : 2023/10/17
* Description        : ����������������ͷ�ļ�
* ********************************************************************************
* Copyright(c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention : This software(modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __TOUCH_KEY_CFG_H__
#define __TOUCH_KEY_CFG_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef enum _TKY_QUEUE_ID
{
TKY_QUEUE_0 = 0,
TKY_QUEUE_1,
TKY_QUEUE_2,
TKY_QUEUE_3,
TKY_QUEUE_4,
TKY_QUEUE_5,

/* Add new above this */
TKY_QUEUE_END
} TKY_QUEUE_ID;

#define TKY_FILTER_MODE     			            3
#define TKY_FILTER_GRADE     			            6
#define TKY_BASE_REFRESH_ON_PRESS     			    0
#define TKY_BASE_UP_REFRESH_DOUBLE     			    0
#define TKY_BASE_DOWN_REFRESH_SLOW     			    0
#define TKY_BASE_REFRESH_SAMPLE_NUM     			1000
#define TKY_SHIELD_EN     			                0
#define TKY_SINGLE_PRESS_MODE     			        1
#define TKY_MAX_QUEUE_NUM     			            6

#define GEN_TKY_CH_INIT(qNum,chNum,chTime,disChTime,slpmode,chBaseline,trs,trs2) \
    {\
     .queueNum=qNum,.channelNum=chNum,.chargeTime=chTime,.disChargeTime=disChTime,\
     .sleepStatus=slpmode,\
     .baseLine = chBaseline,\
     .threshold=trs,\
     .threshold2=trs2\
    }

#define TKY_CHS_INIT \
        GEN_TKY_CH_INIT( TKY_QUEUE_0,  8, 2, 3, 0, 3621, 30, 20 ),\
        GEN_TKY_CH_INIT( TKY_QUEUE_1,  9, 2, 3, 0, 3600, 30, 20 ),\
        GEN_TKY_CH_INIT( TKY_QUEUE_2,  3, 1, 3, 0, 3360, 30, 20 ),\
        GEN_TKY_CH_INIT( TKY_QUEUE_3,  1, 2, 3, 0, 3233, 30, 20 ),\
        GEN_TKY_CH_INIT( TKY_QUEUE_4,  4, 2, 3, 0, 3604, 30, 20 ),\
        GEN_TKY_CH_INIT( TKY_QUEUE_5,  5, 2, 3, 0, 3214, 30, 20 ),\


        /* Add new above this */

//***********************************************************
#ifdef __cplusplus
}
#endif

#endif /* __TOUCH_KEY_CFG_H__ */
