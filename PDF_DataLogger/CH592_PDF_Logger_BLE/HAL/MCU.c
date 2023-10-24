/********************************** (C) COPYRIGHT *******************************
 * File Name          : MCU.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2022/01/18
 * Description        : 硬件任务处理函数及BLE和硬件初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "HAL.h"
#include "spi_flash.h"
#include "peripheral.h"
#include "usb_connect.h"
#include "flash_info.h"

//#define CONFIG_MCU_DEBUG
#ifdef CONFIG_MCU_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif



#define IFLASH_TEMP_START_ADDR   (268*1024)
#define IFLASH_HUMI_START_ADDR   (358*1024)


tmosTaskID halTaskID;
uint32_t g_LLE_IRQLibHandlerLocation;
uint8_t FlashClearFlag = 0;
uint16_t FlashSectorCount = 0;

/*******************************************************************************
 * @fn      Lib_Calibration_LSI
 *
 * @brief   内部32k校准
 *
 * @param   None.
 *
 * @return  None.
 */
void Lib_Calibration_LSI(void)
{
    Calibration_LSI(Level_64);
}

#if(defined(BLE_SNV)) && (BLE_SNV == TRUE)
/*******************************************************************************
 * @fn      Lib_Read_Flash
 *
 * @brief   Callback function used for BLE lib.
 *
 * @param   addr - Read start address
 * @param   num - Number of units to read (unit: 4 bytes)
 * @param   pBuf - Buffer to store read data
 *
 * @return  None.
 */
uint32_t Lib_Read_Flash(uint32_t addr, uint32_t num, uint32_t *pBuf)
{
    EEPROM_READ(addr, pBuf, num * 4);
    return 0;
}

/*******************************************************************************
 * @fn      Lib_Write_Flash
 *
 * @brief   Callback function used for BLE lib.
 *
 * @param   addr - Write start address
 * @param   num - Number of units to write (unit: 4 bytes)
 * @param   pBuf - Buffer with data to be written
 *
 * @return  None.
 */
uint32_t Lib_Write_Flash(uint32_t addr, uint32_t num, uint32_t *pBuf)
{
    EEPROM_ERASE(addr, num * 4);
    EEPROM_WRITE(addr, pBuf, num * 4);
    return 0;
}
#endif

/*******************************************************************************
 * @fn      CH59x_BLEInit
 *
 * @brief   BLE 库初始化
 *
 * @param   None.
 *
 * @return  None.
 */
void CH59x_BLEInit(void)
{
    uint8_t     i;
    bleConfig_t cfg;
    if(tmos_memcmp(VER_LIB, VER_FILE, strlen(VER_FILE)) == FALSE)
    {
        PRINT("head file error...\n");
        while(1);
    }

    SysTick_Config(SysTick_LOAD_RELOAD_Msk);
    PFIC_DisableIRQ(SysTick_IRQn);

    g_LLE_IRQLibHandlerLocation = (uint32_t)LLE_IRQLibHandler;
    tmos_memset(&cfg, 0, sizeof(bleConfig_t));
    cfg.MEMAddr = (uint32_t)MEM_BUF;
    cfg.MEMLen = (uint32_t)BLE_MEMHEAP_SIZE;
    cfg.BufMaxLen = (uint32_t)BLE_BUFF_MAX_LEN;
    cfg.BufNumber = (uint32_t)BLE_BUFF_NUM;
    cfg.TxNumEvent = (uint32_t)BLE_TX_NUM_EVENT;
    cfg.TxPower = (uint32_t)BLE_TX_POWER;
#if(defined(BLE_SNV)) && (BLE_SNV == TRUE)
    if((BLE_SNV_ADDR + BLE_SNV_BLOCK * BLE_SNV_NUM) > (0x78000 - FLASH_ROM_MAX_SIZE))
    {
        PRINT("SNV config error...\n");
        while(1);
    }
    cfg.SNVAddr = (uint32_t)BLE_SNV_ADDR;
    cfg.SNVBlock = (uint32_t)BLE_SNV_BLOCK;
    cfg.SNVNum = (uint32_t)BLE_SNV_NUM;
    cfg.readFlashCB = Lib_Read_Flash;
    cfg.writeFlashCB = Lib_Write_Flash;
#endif
    cfg.ConnectNumber = (PERIPHERAL_MAX_CONNECTION & 3) | (CENTRAL_MAX_CONNECTION << 2);
    cfg.srandCB = SYS_GetSysTickCnt;
#if(defined TEM_SAMPLE) && (TEM_SAMPLE == TRUE)
    cfg.tsCB = HAL_GetInterTempValue; // 根据温度变化校准RF和内部RC( 大于7摄氏度 )
  #if(CLK_OSC32K)
    cfg.rcCB = Lib_Calibration_LSI; // 内部32K时钟校准
  #endif
#endif
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    cfg.idleCB = CH59x_LowPower; // 启用睡眠
#endif
#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
    for(i = 0; i < 6; i++)
    {
        cfg.MacAddr[i] = MacAddr[5 - i];
    }
#else
    {
        uint8_t MacAddr[6];
        GetMACAddress(MacAddr);
        for(i = 0; i < 6; i++)
        {
            cfg.MacAddr[i] = MacAddr[i]; // 使用芯片mac地址
        }
    }
#endif
    if(!cfg.MEMAddr || cfg.MEMLen < 4 * 1024)
    {
        while(1);
    }
    i = BLE_LibInit(&cfg);
    if(i)
    {
        PRINT("LIB init error code: %x ...\n", i);
        while(1);
    }
}

/*******************************************************************************
 * @fn      HAL_ProcessEvent
 *
 * @brief   硬件层事务处理
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events  - events to process.  This is a bit map and can
 *                      contain more than one event.
 *
 * @return  events.
 */
tmosEvents HAL_ProcessEvent(tmosTaskID task_id, tmosEvents events)
{
    uint8_t *msgPtr;

    if(events & SYS_EVENT_MSG)
    { // 处理HAL层消息，调用tmos_msg_receive读取消息，处理完成后删除消息。
        msgPtr = tmos_msg_receive(task_id);
        if(msgPtr)
        {
            /* De-allocate */
            tmos_msg_deallocate(msgPtr);
        }
        return events ^ SYS_EVENT_MSG;
    }

    if(events & HAL_REG_INIT_EVENT)
    {
#if(defined BLE_CALIBRATION_ENABLE) && (BLE_CALIBRATION_ENABLE == TRUE) // 校准任务，单次校准耗时小于10ms
        BLE_RegInit();                                                  // 校准RF
  #if(CLK_OSC32K)
        Lib_Calibration_LSI(); // 校准内部RC
  #endif
        tmos_start_task(halTaskID, HAL_REG_INIT_EVENT, MS1_TO_SYSTEM_TIME(BLE_CALIBRATION_PERIOD));
        return events ^ HAL_REG_INIT_EVENT;
#endif
    }

    if(events & HAL_AHT20_EVENT)
    {
        static int16_t dat[2];

        HAL_AHT20_MeasureReadTemperatureHumidity();

        if(DeviceInfo.MeasureNum < MEASURENT_DATA_MAX)
        {
            dat[0] = (int16_t)(AHT20_TemperatureValue*10);
            dat[1] = (int16_t)(AHT20_HumidityValue*10);

            LOG_INFO("T:%d | H:%d\n", dat[0], dat[1]);

            FLASH_ROM_WRITE(IFLASH_TEMP_START_ADDR+4*DeviceInfo.MeasureNum, dat, 4);
//            FLASH_ROM_WRITE(IFLASH_HUMI_START_ADDR+4*DeviceInfo.MeasureNum, &AHT20_HumidityValue, 4);
            ++DeviceInfo.MeasureNum;
            HAL_SaveDeviceInfo();
            PRINT("MeasureNum:%d\n", DeviceInfo.MeasureNum);
        }

        tmos_set_event(Peripheral_TaskID, SBP_UPDATE_SCAN_RESPONSE_EVT);
        tmos_start_task(halTaskID, HAL_AHT20_EVENT, MS1_TO_SYSTEM_TIME(DeviceInfo.PdfParam.MeasureInterval*60*1000));

        return events ^ HAL_AHT20_EVENT;
    }

    if(events & HAL_LCD_EVENT)
    {
        if (LCD_DisplayFlg == 0)
        {
            HAL_LCD_ShowTemperature(AHT20_TemperatureValue);
            LCD_DisplayFlg = 1;
        }
        else
        {
            HAL_LCD_ShowHumidity(AHT20_HumidityValue);
            LCD_DisplayFlg = 0;
        }

        tmos_start_task(halTaskID, HAL_LCD_EVENT, MS1_TO_SYSTEM_TIME(2000));
        return events ^ HAL_LCD_EVENT;
    }

    if (events & HAL_KEY_EVENT)
    {
        HAL_KeyPoll(); /* Check for keys */
        tmos_start_task(halTaskID, HAL_KEY_EVENT, MS1_TO_SYSTEM_TIME(HAL_KEY_POLLING_PERIOD));

        return (events ^ HAL_KEY_EVENT);
    }

    if (events & HAL_USB_CHECK_EVENT)
    {
        if(USB_CONNECT_CHECK() != 0)
        {
            tmos_stop_task(halTaskID, HAL_KEY_EVENT);
            tmos_clear_event(halTaskID, HAL_KEY_EVENT);

            tmos_stop_task(halTaskID, HAL_LCD_EVENT);
            tmos_clear_event(halTaskID, HAL_LCD_EVENT);
            HAL_LCD_ClearScreen();

            tmos_stop_task(halTaskID, HAL_AHT20_EVENT);
            tmos_clear_event(halTaskID, HAL_AHT20_EVENT);

            tmos_start_task(Peripheral_TaskID, SBP_DISABLE_ADV_EVT, MS1_TO_SYSTEM_TIME(20));
            tmos_start_task(halTaskID, HAL_DEVICE_RESET_EVENT, MS1_TO_SYSTEM_TIME(100));
        }
        else
        {
            tmos_start_task(halTaskID, HAL_USB_CHECK_EVENT, MS1_TO_SYSTEM_TIME(100));
        }

        return (events ^ HAL_USB_CHECK_EVENT);
    }

    if (events & HAL_DEVICE_RESET_EVENT)
    {
        PRINT("Reset\n");
        DelayMs(10);
        SYS_ResetExecute();

        return (events ^ HAL_DEVICE_RESET_EVENT);
    }

    if (events & HAL_SPI_FLASH_CLEAR_EVENT)
    {
        FLASH_Erase_Sector(4096*FlashSectorCount);
        ++FlashSectorCount;
        if(FlashSectorCount<20)
        {
            tmos_start_task(halTaskID, HAL_SPI_FLASH_CLEAR_EVENT, MS1_TO_SYSTEM_TIME(20));
        }
        else
        {
            FlashSectorCount = 1;
            tmos_start_task(halTaskID, HAL_CODE_FLASH_CLEAR_EVENT, MS1_TO_SYSTEM_TIME(20));
        }

        return (events ^ HAL_SPI_FLASH_CLEAR_EVENT);
    }

    if (events & HAL_CODE_FLASH_CLEAR_EVENT)
    {
        FLASH_ROM_ERASE(MEASURENT_DATA_ADDR+4096*FlashSectorCount, 4096);
        ++FlashSectorCount;
        if(FlashSectorCount < MEASURENT_DATA_FLASH_SIZE/4096)
        {
            tmos_start_task(halTaskID, HAL_CODE_FLASH_CLEAR_EVENT, MS1_TO_SYSTEM_TIME(20));
        }
        else
        {
            FlashSectorCount = 0;
            FlashClearFlag = 1;
        }

        return (events ^ HAL_CODE_FLASH_CLEAR_EVENT);
    }

    return 0;
}

/*******************************************************************************
 * @fn      HAL_Init
 *
 * @brief   硬件初始化
 *
 * @param   None.
 *
 * @return  None.
 */
void HAL_Init()
{
    halTaskID = TMOS_ProcessEventRegister(HAL_ProcessEvent);
    HAL_TimeInit();

#if(defined BLE_CALIBRATION_ENABLE) && (BLE_CALIBRATION_ENABLE == TRUE)
    tmos_start_task(halTaskID, HAL_REG_INIT_EVENT, MS1_TO_SYSTEM_TIME(BLE_CALIBRATION_PERIOD)); // 添加校准任务，单次校准耗时小于10ms
#endif

    HAL_AHT20_Init();
    HAL_LCD_Init(LCD_1_4_Duty, LCD_1_3_Bias);
    HAL_KeyInit();

    GPIOA_ModeCfg(USB_CHECK_BV, GPIO_ModeIN_PD);
    tmos_start_task(halTaskID, HAL_USB_CHECK_EVENT, MS1_TO_SYSTEM_TIME(100));

    tmos_start_task(halTaskID, HAL_KEY_EVENT, MS1_TO_SYSTEM_TIME(20));
//    tmos_start_task(halTaskID, HAL_AHT20_EVENT, MS1_TO_SYSTEM_TIME(100));
//    tmos_start_reload_task(halTaskID, HAL_LCD_EVENT, MS1_TO_SYSTEM_TIME(2000));
}

/*******************************************************************************
 * @fn      HAL_GetInterTempValue
 *
 * @brief   获取内部温感采样值，如果使用了ADC中断采样，需在此函数中暂时屏蔽中断.
 *
 * @return  内部温感采样值.
 */
uint16_t HAL_GetInterTempValue(void)
{
    uint8_t  sensor, channel, config, tkey_cfg;
    uint16_t adc_data;

    tkey_cfg = R8_TKEY_CFG;
    sensor = R8_TEM_SENSOR;
    channel = R8_ADC_CHANNEL;
    config = R8_ADC_CFG;
    ADC_InterTSSampInit();
    R8_ADC_CONVERT |= RB_ADC_START;
    while(R8_ADC_CONVERT & RB_ADC_START);
    adc_data = R16_ADC_DATA;
    R8_TEM_SENSOR = sensor;
    R8_ADC_CHANNEL = channel;
    R8_ADC_CFG = config;
    R8_TKEY_CFG = tkey_cfg;
    return (adc_data);
}

/******************************** endfile @ mcu ******************************/
