/********************************** (C) COPYRIGHT *******************************
* File Name          : KEY.c
* Author             : WCH
* Version            : V1.0
* Date               : 2014/05/12
* Description        :
*******************************************************************************/

/******************************************************************************/
#include "HAL.h"
#include "flash_info.h"
#include "peripheral.h"
#include "spi_flash.h"

//#define CONFIG_KEY_DEBUG
#ifdef CONFIG_KEY_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
/* Save the previously scanned key value for key debounce */
static uint8_t halPrevScanKeys = 0;

/* Save the previous valid key value to determine whether there is a change in the key value */
static uint8_t halPrevValidKeys = 0;

/* Key has been lifted sign (the key pressed when turning on is lifted) */
//static uint8_t KeyUpOnceFlag = 1;


static uint8_t halPentaClickFlag = 0; //计数按键按下的次数标志

static uint16_t shutdown_time_count = 0;

//
///* 准备进入shutdown标志 */
//uint8_t SHUTDOWN_FLAG = 0;
//
///* 按键空闲时间 */
//uint16_t KeyIdleTime = 0;
//
///* 按键空闲开启睡眠时间，默认为5s */
//uint16_t KeyIdleTimeout = 5;
//
///* 按键轮询的标志标志 */
//uint8_t KeyPollEnFlag = 0;



/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
static halKeyCBack_t pHalKeyProcessFunction;        /* callback function */

/**************************************************************************************************
 * @fn      HAL_KeyInit
 *
 * @brief   Initilize Key Service
 *
 * @param   none
 *
 * @return  None
 **************************************************************************************************/
void HAL_KeyInit(void)
{
    KEY1_DIR;
    KEY1_PU;

    pHalKeyProcessFunction = HAL_KeyProcessFunction_FactoryStatus;
//    KeyPollEnFlag = 0;
}

/**************************************************************************************************
 * @fn      HAL_KeyPoll
 *
 * @brief   Called by hal_driver to poll the keys
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_KeyPoll (void)
{
    uint8_t ScanKeys = 0;                       //Scanned keys

//    static uint16_t shutdown_time_count = 0;

    if (HAL_PUSH_BUTTON1())
    {
        ScanKeys |= HAL_KEY_SW_1;
    }

    /* Invoke Callback if new keys were depressed */
    if ((halPrevScanKeys == ScanKeys) && (pHalKeyProcessFunction)) //If the key value is the same twice, the key value is valid.
    {
        (pHalKeyProcessFunction) (ScanKeys);
    }
    halPrevScanKeys = ScanKeys; //Save key value

    if(halPentaClickFlag > 0)
    {
        if(shutdown_time_count > (4000/HAL_KEY_POLLING_PERIOD))
        {
            shutdown_time_count = 0;
            halPentaClickFlag = 0; //短按时间清零
            LOG_INFO("Clear shutdown time count\n");
        }
        else
        {
            ++shutdown_time_count;
        }
    }
}

/**************************************************************************************************
 * @fn      HAL_KeyProcessFunction_FactoryStatus
 *
 * @brief   Key processing in factory mode
 *
 * @param   keys - Keys after debounce
 *
 * @return  None
 **************************************************************************************************/
void HAL_KeyProcessFunction_FactoryStatus(uint8_t keys)
{
    static uint16_t wakeup_time_count = 0;      //Wakeup key press time count

    if (keys & HAL_KEY_SW_1)
    {
        if (halPrevValidKeys & HAL_KEY_SW_1)    //Key continues to be pressed
        {
            ++wakeup_time_count;
        }
        else                                    //Key is pressed for the first time
        {
            wakeup_time_count = 1;
        }

        if(wakeup_time_count > (3000/HAL_KEY_POLLING_PERIOD)) //Key is pressed continuously for 3 seconds
        {
            LOG_INFO("Wake Up success\n");
            wakeup_time_count = 0;
            DeviceStatus = DEF_DEVICE_STATUS_NORMAL;

//            for(uint16_t i=0; i<20; ++i)
//            {
//                FLASH_Erase_Sector(4096*i);
//            }
//            FLASH_ROM_ERASE(MEASURENT_DATA_ADDR, MEASURENT_DATA_FLASH_SIZE);
            FLASH_ROM_ERASE(MEASURENT_DATA_ADDR, 4096);
            FlashClearFlag = 0;
            FlashSectorCount = 0;
            tmos_start_task(halTaskID, HAL_SPI_FLASH_CLEAR_EVENT, MS1_TO_SYSTEM_TIME(50));
            tmos_start_task(Peripheral_TaskID, SBP_ENABLE_ADV_EVT, MS1_TO_SYSTEM_TIME(20));
            tmos_start_task(halTaskID, HAL_AHT20_EVENT, MS1_TO_SYSTEM_TIME(100));
            tmos_start_task(halTaskID, HAL_LCD_EVENT, MS1_TO_SYSTEM_TIME(200));

            DeviceInfo.StatusFlag = DEF_DEVICE_STATUS_NORMAL;
            DeviceInfo.MeasureNum = 0;
            HAL_SaveDeviceInfo();
        }
    }
    else if (halPrevValidKeys & HAL_KEY_SW_1)         //key1按下并抬起
    {
        if (DeviceStatus == DEF_DEVICE_STATUS_FACTORY)
        {
            LOG_INFO("Wake Up fail\n");
            wakeup_time_count =0;
        }
        else
        {
            LOG_INFO("Wake up key release once\n");

            pHalKeyProcessFunction = HAL_KeyProcessFunction_NormalStatus;
        }
    }

    halPrevValidKeys = keys; //保存有效键值
}


void HAL_KeyProcessFunction_NormalStatus(uint8_t keys)
{
    if (((keys & HAL_KEY_SW_1) == 0) && ((halPrevValidKeys & HAL_KEY_SW_1) != 0))         //key1按下并抬起
    {
        if(halPentaClickFlag == 4)
        {
            halPentaClickFlag = 0;
            DeviceStatus = DEF_DEVICE_STATUS_FACTORY;
            LOG_INFO("Penta clik\n"); //按键间隔4s以内有效

            tmos_start_task(Peripheral_TaskID, SBP_DISABLE_ADV_EVT, MS1_TO_SYSTEM_TIME(20));

            tmos_stop_task(halTaskID, HAL_LCD_EVENT);
            tmos_clear_event(halTaskID, HAL_LCD_EVENT);
            HAL_LCD_ClearScreen();
//
            tmos_stop_task(halTaskID, HAL_AHT20_EVENT);
            tmos_clear_event(halTaskID, HAL_AHT20_EVENT);

            pHalKeyProcessFunction = HAL_KeyProcessFunction_FactoryStatus;

            DeviceInfo.StatusFlag = DEF_DEVICE_STATUS_FACTORY;
            HAL_SaveDeviceInfo();
        }
        else
        {
            ++halPentaClickFlag;
        }
    }

    halPrevValidKeys = keys; //保存有效键值
}

//void HAL_KeyProcessFunction_FactoryStatus(uint8_t keys)
//{
//    if (halPrevScanKeys == keys) //If the key value is the same twice, the key value is valid.
//    {
//        if (ScanKeys & HAL_KEY_SW_1)
//        {
//            KeyIdleTime = 0;
//            if (WakeupFlag == 1)
//            {
//                if (halPrevValidKeys & HAL_KEY_SW_1)   { ++wakeup_time_count; } //Key continues to be pressed
//                else    { wakeup_time_count = 1; }  //Key is pressed for the first time
//
//                if(wakeup_time_count > (5000/HAL_KEY_POLLING_PERIOD)) //Key is pressed continuously for 5 seconds
//                {
//                    LOG_INFO("Wake Up success\n");
//                    wakeup_time_count = 0;
//                    WakeupFlag = 0;
//                    KeyUpOnceFlag = 0;
////                    tmos_start_task(Broadcaster_TaskID, SBP_ADV_ENABLE_EVT, MS1_TO_SYSTEM_TIME(20));
////                    tmos_start_task(halTaskID, SBP_BAT_VOL_GET_EVT, MS1_TO_SYSTEM_TIME(5000));
//                }
//            }
//        }
//        else if (halPrevValidKeys & HAL_KEY_SW_1)         //key1按下并抬起
//        {
//            KeyIdleTime = 0;
//            if (WakeupFlag == 1)
//            {
//                LOG_INFO("Wake Up fail\n");
//                wakeup_time_count =0;
////                tmos_set_event(halTaskID, HAL_SHUTDOWN_EVENT);
//            }
//            else if (WakeupFlag == 0)
//            {
//                LOG_INFO("Wake up key release once\n");
//                WakeupFlag = 1;
//            }
////            else if (SHUTDOWN_FLAG == 0)
////            {
////                if(halPentaClickFlag == 4)
////                {
////                    halPentaClickFlag = 0;
////                    LOG_INFO("Penta clik\n"); //按键间隔4s以内有效
////                    SHUTDOWN_FLAG = 1;
////                    MyHalLedControl.times += 5;
////                    tmos_stop_task(halTaskID, SBP_BAT_VOL_GET_EVT);
////                    tmos_clear_event(halTaskID, SBP_BAT_VOL_GET_EVT);
////                    tmos_start_task(halTaskID, HAL_SHUTDOWN_EVENT, MS1_TO_SYSTEM_TIME(6200)); //灯闪烁完成后睡眠
////                }
////                else
////                {
////                    ++halPentaClickFlag;
////                }
////            }
//        }
//
//        halPrevValidKeys = ScanKeys; //保存有效键值
//    }
//
//
////    if(halPentaClickFlag > 0)
////    {
////        if(shutdown_time_count > (4000/HAL_KEY_POLLING_PERIOD))
////        {
////            shutdown_time_count = 0;
////            halPentaClickFlag = 0; //短按时间清零
////#if DEBUG1
////            PRINT("Clear shutdown time count\n");
////#endif
////        }
////        else
////        {
////            ++shutdown_time_count;
////        }
////    }
//}

/******************************** endfile @ key ******************************/
