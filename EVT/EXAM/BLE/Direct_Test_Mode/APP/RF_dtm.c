/********************************** (C) COPYRIGHT *******************************
 * File Name          : RF_dtm.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2025/06/30
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include <RF_dtm.h>
#include "CONFIG.h"
#include "RingMem.h"
#include "app_usb.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define RF_SEND_DELAY        0         // 单位us，用于控制发包速率(包间隔)

uint8_t taskID;
volatile uint8_t tx_end_flag=0;
volatile uint8_t rx_end_flag=0;

__attribute__((__aligned__(4))) uint8_t TxBuf[255]; //最长发送字节255
__attribute__((__aligned__(4))) uint8_t RxBuf[264]; // 接收DMA buf不能小于264字节

uint8_t uartRingMemBuff[1024];
RingMemParm uartRingParm = {0};
uint8_t usbRingMemBuff[1024];
RingMemParm usbRingParm = {0};

const uint8_t endTestCmd[4]={0x01, 0x1F, 0x20, 0x00};
const uint8_t resetCmd[4]={0x01, 0x03, 0x0C, 0x00};
const uint8_t receiverCmd[4]={0x01, 0x1D, 0x20, 0x01};
const uint8_t transmitterCmd[4]={0x01, 0x1E, 0x20, 0x03};
const uint8_t receiver2MCmd[4]={0x01, 0x33};
const uint8_t transmitter2MCmd[4]={0x01, 0x34};
const uint8_t setPowerCmd[4]={0x01, 0x01, 0xFF, 0x01};
const uint8_t pFCmd[4]={0x01, 0x02, 0xFF, 0x01};
const uint8_t SingleCarrierCmd[4]={0x01, 0x03, 0xFF, 0x01};
uint8_t cmdCompleteEvt[23]={0x04, 0x0E};
uint8_t TEST_MODE= 0xFF;   // 测试模式，发送或者接收
uint8_t ttflag=0; //发送完成标志
uint8_t ch;

rfConfig_t rf_Config;

#define  MODE_RX     0
#define  MODE_TX     1

uint32_t volatile gTxCount;
uint32_t volatile gRxCount;

void TX_DATA( uint8_t *buf, uint8_t len );

/*********************************************************************
 * @fn      RF_Wait_Tx_End
 *
 * @brief   手动模式等待发送完成，自动模式等待发送-接收完成，必须在RAM中等待，等待时可以执行用户代码，但需要注意执行的代码必须运行在RAM中，否则影响发送
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void RF_Wait_Tx_End()
{
    uint32_t i=0;
    while(!tx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // 约5ms超时
        if(i>(FREQ_SYS/1000))
        {
            tx_end_flag = TRUE;
        }
    }
}

/*********************************************************************
 * @fn      RF_Wait_Rx_End
 *
 * @brief   自动模式等待应答发送完成，必须在RAM中等待，等待时可以执行用户代码，但需要注意执行的代码必须运行在RAM中，否则影响发送
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void RF_Wait_Rx_End()
{
    uint32_t i=0;
    while(!rx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // 约5ms超时
        if(i>(FREQ_SYS/1000))
        {
            rx_end_flag = TRUE;
        }
    }
}

/*******************************************************************************
 * @fn      m_UART_SendString
 *
 * @brief   uart发送函数
 *
 * @param   *buf - 发送buffer.
 *
 * @return  None.
 */
__HIGH_CODE
void m_UART_SendString(uint8_t *buf, uint16_t l)
{
    uint16_t len = l;

    while(len)
    {
        if(R8_UART1_TFC != UART_FIFO_SIZE)
        {
            R8_UART1_THR = *buf++;
            len--;
        }
    }
}

/*******************************************************************************
 * @fn      m_UART_RecvString
 *
 * @brief   uart接收函数
 *
 * @param   *buf - 接收buffer.
 *
 * @return  None.
 */
__HIGH_CODE
uint16_t m_UART_RecvString(uint8_t *buf)
{
    uint16_t len = 0;

    while(R8_UART1_RFC)
    {
        *buf++ = R8_UART1_RBR;
        len++;
    }

    return (len);
}

/*******************************************************************************
 * @brief   rf发送数据子程序
 *
 * @param   pBuf - 发送的DMA地址
 *
 * @return  None.
 */
__HIGH_CODE
void rf_tx_start( uint8_t *pBuf )
{
    RF_Shut();
    tx_end_flag = FALSE;
    if(pBuf[1] > 251)  pBuf[1] = 251;   // RF_Tx最大只能发251
    if(!RF_Tx( &pBuf[2], pBuf[1], 0xFF, 0xFF))
    {
        RF_Wait_Tx_End();
        if(RF_SEND_DELAY)   DelayUs(RF_SEND_DELAY);
    }
}

/*******************************************************************************
 * @fn      rf_rx_start
 *
 * @brief   rf接收数据子程序
 *
 * @return  None.
 */
__HIGH_CODE
void rf_rx_start( void )
{
    RF_Rx(RxBuf, 40, 0xFF, 0xFF);
}

/*********************************************************************
 * @fn      RF_2G4StatusCallBack
 *
 * @brief   RF 状态回调，此函数在中断中调用。注意：不可在此函数中直接调用RF接收或者发送API，需要使用事件的方式调用
 *          在此回调中直接使用或调用函数涉及到的变量需注意，此函数在中断中调用。
 *
 * @param   sta     - 状态类型
 * @param   crc     - crc校验结果
 * @param   rxBuf   - 数据buf指针
 *
 * @return  none
 */
__HIGH_CODE
void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
    {
        case TX_MODE_TX_FINISH:
        {
#if(!RF_AUTO_MODE_EXAM)
            tx_end_flag = TRUE;
#endif
            if( TEST_MODE ==  MODE_TX )
            {
                gTxCount ++;
                ttflag = 1;
            }
            break;
        }
        case TX_MODE_TX_FAIL:
        {
            tx_end_flag = TRUE;
            break;
        }
        case TX_MODE_RX_DATA:
        {
#if(RF_AUTO_MODE_EXAM)
            tx_end_flag = TRUE;
            if (crc == 0) {
                uint8_t i;

                PRINT("tx recv,rssi:%d\n", (int8_t)rxBuf[0]);
                PRINT("len:%d-", rxBuf[1]);

                for (i = 0; i < rxBuf[1]; i++) {
                    PRINT("%x ", rxBuf[i + 2]);
                }
                PRINT("\n");
            } else {
                if (crc & (1<<0)) {
                    PRINT("crc error\n");
                }

                if (crc & (1<<1)) {
                    PRINT("match type error\n");
                }
            }
#endif
            break;
        }
        case TX_MODE_RX_TIMEOUT: // Timeout is about 200us
        {
            if( TEST_MODE ==  MODE_RX )
            {
              rf_rx_start( );
            }
#if(RF_AUTO_MODE_EXAM)
            tx_end_flag = TRUE;
#endif
            break;
        }
        case RX_MODE_RX_DATA:
        {
            if (crc == 0) {
                uint8_t i;
#if(RF_AUTO_MODE_EXAM)
                RF_Wait_Rx_End();
#endif
                gRxCount ++;
                rf_rx_start();
            } else {
                if (crc & (1<<0)) {
                    rf_rx_start();
                }

                if (crc & (1<<1)) {
                    rf_rx_start();
                }
            }
#if(!RF_AUTO_MODE_EXAM)
            tmos_set_event(taskID, SBP_RF_RX_EVT);
#endif
            break;
        }
        case RX_MODE_TX_FINISH:
        {
#if(RF_AUTO_MODE_EXAM)
            rx_end_flag = TRUE;
            tmos_set_event(taskID, SBP_RF_RX_EVT);
#endif
            break;
        }
        case RX_MODE_TX_FAIL:
        {
#if(RF_AUTO_MODE_EXAM)
            rx_end_flag = TRUE;
            tmos_set_event(taskID, SBP_RF_RX_EVT);
#endif
            break;
        }
    }
}

/*********************************************************************
 * @fn      RF_ProcessEvent
 *
 * @brief   RF 事件处理
 *
 * @param   task_id - 任务ID
 * @param   events  - 事件标志
 *
 * @return  未完成事件
 */
__HIGH_CODE
uint16_t RF_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if(events & SBP_RF_PERIODIC_EVT)
    {
        return events ^ SBP_RF_PERIODIC_EVT;
    }
    if(events & SBP_RF_TX_EVT)
    {
        rf_tx_start(TxBuf);
        return events ^ SBP_RF_TX_EVT;
    }
    if(events & SBP_RF_RX_EVT)
    {

        return events ^ SBP_RF_RX_EVT;
    }
    return 0;
}

/*********************************************************************
 * @fn      RF_Init
 *
 * @brief   RF 初始化
 *
 * @return  none
 */
void RF_Init(void)
{
    uint8_t    state;
//    rfConfig_t rf_Config;
    RingMemInit( &uartRingParm, uartRingMemBuff, sizeof(uartRingMemBuff) );
    RingMemInit( &usbRingParm, usbRingMemBuff, sizeof(usbRingMemBuff) );

    tmos_memset(&rf_Config, 0, sizeof(rfConfig_t));
    taskID = TMOS_ProcessEventRegister(RF_ProcessEvent);
    rf_Config.accessAddress = 0x71764129; // 禁止使用0x55555555以及0xAAAAAAAA ( 建议不超过24次位反转，且不超过连续的6个0或1 )
    rf_Config.CRCInit = 0x555555;
    rf_Config.Channel = 39;
    rf_Config.Frequency = 2480000;
#if(RF_AUTO_MODE_EXAM)
    rf_Config.LLEMode = LLE_MODE_AUTO;
#else
    rf_Config.LLEMode = LLE_MODE_BASIC | LLE_WHITENING_OFF; // 使能 LLE_MODE_EX_CHANNEL 表示 选择 rf_Config.Frequency 作为通信频点
#endif
    rf_Config.rfStatusCB = RF_2G4StatusCallBack;
    rf_Config.RxMaxlen = 251;
    state = RF_Config(&rf_Config);
    PRINT("rf 2.4g init: %x\n", state);
}

/*******************************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   串口1中断函数
 *
 * @param   None.
 *
 * @return  None.
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
__HIGH_CODE
void UART1_IRQHandler()
{
  if( (R8_UART1_IIR & RB_IIR_INT_MASK) !=1 )
  {
    switch( R8_UART1_IIR & RB_IIR_INT_MASK )
    {
        case UART_II_RECV_RDY:       //接收数据可用
        case UART_II_RECV_TOUT:      //接收超时
        {
            uint8_t uartRecBuff[10];
            uint8_t uartRecLen;
            uartRecLen = m_UART_RecvString( uartRecBuff );
            // 写入缓冲区
            if( RingMemWrite( &uartRingParm, uartRecBuff, uartRecLen ) )
            {
                PRINT("RingMem err %d %d %d \n",uartRecLen,uartRingParm.RemanentLen,uartRingParm.MaxLen);
            }
            break;
        }
        default:
            break;
    }
  }
}

/*******************************************************************************
 * @fn      Choose_CH
 *
 * @brief   rf通道转换为BLE通道
 *
 * @param   rf通道.
 *
 * @return  BLE通道.
 */
__HIGH_CODE
uint8_t Choose_CH( uint8_t cch )
{
  if( cch == 0 )
    return 37;
  else if( cch == 12 )
    return 38;
  else if( cch == 39 )
    return 39;
  else if( cch < 12 )
    return cch-1;
  else if( cch > 12 )
    return cch-2;
  else
    return 0;
}

/*******************************************************************************
 * @fn      UART_Process_Data
 *
 * @brief   串口信令处理
 *
 * @param   None.
 *
 * @return  None.
 */
uint8_t single =0;
__HIGH_CODE
void UART_Process_Data(void)
{
 if( uartRingParm.CurrentLen >=4  )
 {
    uint8_t pData[23];
    uint8_t dataLen;
    if( RingReturnSingleData( &uartRingParm, 0 )!=0x01 )
    {
      RingMemDelete( &uartRingParm, 1 );
      return;
    }
    dataLen = RingReturnSingleData( &uartRingParm, 3 );
    if( uartRingParm.CurrentLen < dataLen+4 )
    {
      return;
    }
    RingMemCopy( &uartRingParm, pData, dataLen+4 );
    if( __wrap_memcmp( pData, endTestCmd, 4 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x06;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      if( single == 1)
      {
        LL_TestEnd(NULL);
        single = 0;
      }
      if( TEST_MODE == MODE_TX )
      {
        cmdCompleteEvt[7] = (gTxCount)&0xFF;
        cmdCompleteEvt[8] = ((gTxCount)>>8)&0xFF;
        m_UART_SendString( cmdCompleteEvt, 9 );
        ch=0;
        gRxCount = 0;
        gTxCount = 0;
        TEST_MODE = 0xFF;
      }
      else
      {//rx
        cmdCompleteEvt[7] = (gRxCount)&0xFF;
        cmdCompleteEvt[8] = ((gRxCount)>>8)&0xFF;
        m_UART_SendString( cmdCompleteEvt, 9 );
        ch=0;
        gRxCount = 0;
        gTxCount = 0;
        TEST_MODE = 0xFF;
      }
    }
    else if( __wrap_memcmp( pData, SingleCarrierCmd, 4 ) == 0)
    {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      ch = pData[4];
//      ch = Choose_CH(pData[4]);
      single = 1;
      LL_SingleChannel(ch);
    }
    else if( __wrap_memcmp( pData, resetCmd, 4 ) == 0)
    {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      ch=0;
      gRxCount = 0;
      gTxCount = 0;
      TEST_MODE = 0xFF;
    }
    else if( __wrap_memcmp( pData, receiverCmd, 4 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);
      gRxCount = 0;
      rf_Config.LLEMode &= ~(3<<4);
      rf_Config.LLEMode |= LLE_MODE_PHY_1M;
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TEST_MODE = MODE_RX;
      rf_rx_start();
    }
    else if( __wrap_memcmp( pData, transmitterCmd, 4 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);
      gRxCount = 0;
      rf_Config.LLEMode &= ~(3<<4);
      rf_Config.LLEMode |= LLE_MODE_PHY_1M;
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TxBuf[1] = pData[5];
      TxBuf[0] = pData[6];
      TEST_MODE = MODE_TX;
      TX_DATA( TxBuf, TxBuf[1] );
      tmos_set_event(taskID, SBP_RF_TX_EVT);
   }
   else if( __wrap_memcmp( pData, setPowerCmd, 4 ) == 0)
   {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      LL_SetTxPowerLevel(pData[4]);
    }
   else if( __wrap_memcmp( pData, pFCmd, 4 ) == 0)
   {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      //电容
      R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1;
      R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;
      R8_XT32M_TUNE &= ~RB_XT32M_C_LOAD;
      R8_XT32M_TUNE |= pData[4]<<4;
      R8_SAFE_ACCESS_SIG = 0;
   }
   else if( __wrap_memcmp( pData, receiver2MCmd, 2 ) == 0)
   {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);

      if( pData[5]==1 )
      {
          rf_Config.LLEMode &= ~(3<<4);
      }
      else if( pData[5]==2 )
      {
          rf_Config.LLEMode |= LLE_MODE_PHY_2M;
      }
      gRxCount = 0;
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TEST_MODE = MODE_RX;
      rf_rx_start();
    }
    else if( __wrap_memcmp( pData, transmitter2MCmd, 2 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);
      TxBuf[1] = pData[5];
      TxBuf[0] = pData[6];
      TX_DATA( TxBuf, TxBuf[1] );
     if( pData[7]==1 )
     {
         rf_Config.LLEMode &= ~(3<<4);
      }
      else if( pData[7]==2 )
      {
          rf_Config.LLEMode |= LLE_MODE_PHY_2M;
      }
      else if( pData[7]==3 )
      {
      }
      else if( pData[7]==4 )
      {
      }
      gRxCount = 0;
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TEST_MODE = MODE_TX;
      tmos_set_event(taskID, SBP_RF_TX_EVT);
    }
    else
    {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      m_UART_SendString( cmdCompleteEvt, 7 );
    }
    RingMemDelete( &uartRingParm, dataLen+4 );
 }
}

/*******************************************************************************
 * @fn      USB_Process_Data
 *
 * @brief   串口信令处理
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void USB_Process_Data(void)
{
 if( usbRingParm.CurrentLen >= 4  )
 {
    uint8_t pData[23];
    uint8 dataLen;
    if( RingReturnSingleData( &usbRingParm, 0 )!=0x01 )
    {
      RingMemDelete( &usbRingParm, 1 );
      return;
    }
    dataLen = RingReturnSingleData( &usbRingParm, 3 );
    if( usbRingParm.CurrentLen < dataLen+4 )
    {
      return;
    }
    RingMemCopy( &usbRingParm, pData, dataLen+4 );

    if( __wrap_memcmp( pData, endTestCmd, 4 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x06;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      if( single == 1)
      {
        LL_TestEnd(NULL);
        single = 0;
      }
      if( TEST_MODE == MODE_TX )
      {
        cmdCompleteEvt[7] = (gTxCount)&0xFF;
        cmdCompleteEvt[8] = ((gTxCount)>>8)&0xFF;
        USBSendData( cmdCompleteEvt, 9 );
        ch=0;
        gRxCount = 0;
        gTxCount = 0;
        TEST_MODE = 0xFF;
      }
      else
      {//rx
        cmdCompleteEvt[7] = (gRxCount)&0xFF;
        cmdCompleteEvt[8] = ((gRxCount)>>8)&0xFF;
        USBSendData( cmdCompleteEvt, 9 );
        ch=0;
        gRxCount = 0;
        gTxCount = 0;
        TEST_MODE = 0xFF;
      }
    }
    else if( __wrap_memcmp( pData, SingleCarrierCmd, 4 ) == 0)
    {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      ch = pData[4];
//      ch = Choose_CH(pData[4]);
      single = 1;
      LL_SingleChannel(ch);
    }
    else if( __wrap_memcmp( pData, resetCmd, 4 ) == 0)
    {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      ch=0;
      gRxCount = 0;
      gTxCount = 0;
      TEST_MODE = 0xFF;
    }
    else if( __wrap_memcmp( pData, receiverCmd, 4 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);
      gRxCount = 0;
      rf_Config.LLEMode &= ~(3<<4);
      rf_Config.LLEMode |= LLE_MODE_PHY_1M;
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TEST_MODE = MODE_RX;
      rf_rx_start();
    }
    else if( __wrap_memcmp( pData, transmitterCmd, 4 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);
      gRxCount = 0;
      rf_Config.LLEMode &= ~(3<<4);
      rf_Config.LLEMode |= LLE_MODE_PHY_1M;
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TxBuf[1] = pData[5];
      TxBuf[0] = pData[6];
      TEST_MODE = MODE_TX;
      TX_DATA( TxBuf, TxBuf[1] );
      tmos_set_event(taskID, SBP_RF_TX_EVT);
   }
   else if( __wrap_memcmp( pData, setPowerCmd, 4 ) == 0)
   {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      LL_SetTxPowerLevel(pData[4]);
    }
   else if( __wrap_memcmp( pData, pFCmd, 4 ) == 0)
   {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      //电容
      R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1;
      R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;
      R8_XT32M_TUNE &= ~RB_XT32M_C_LOAD;
      R8_XT32M_TUNE |= pData[4]<<4;
      R8_SAFE_ACCESS_SIG = 0;
   }
   else if( __wrap_memcmp( pData, receiver2MCmd, 2 ) == 0)
   {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);

      if( pData[5]==1 )
      {
          rf_Config.LLEMode &= ~(3<<4);
      }
      else if( pData[5]==2 )
      {
          rf_Config.LLEMode |= LLE_MODE_PHY_2M;
      }
      gRxCount = 0;
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TEST_MODE = MODE_RX;
      rf_rx_start();
    }
    else if( __wrap_memcmp( pData, transmitter2MCmd, 2 ) == 0)
    {
      RF_Shut();
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
      ch = Choose_CH(pData[4]);
      TxBuf[1] = pData[5];
      TxBuf[0] = pData[6];
      TX_DATA( TxBuf, TxBuf[1] );
     if( pData[7]==1 )
     {
          rf_Config.LLEMode &= ~(3<<4);
      }
      else if( pData[7]==2 )
      {
          rf_Config.LLEMode |= LLE_MODE_PHY_2M;
      }
      else if( pData[7]==3 )
      {
      }
      else if( pData[7]==4 )
      {
      }
      gRxCount = 0;

      PRINT("%d\n",ch);
      rf_Config.Channel = ch;
      RF_Config(&rf_Config);
      TEST_MODE = MODE_TX;
      tmos_set_event(taskID, SBP_RF_TX_EVT);
    }
    else
    {
      cmdCompleteEvt[2] = 0x04;
      cmdCompleteEvt[3] = 0x01;
      cmdCompleteEvt[4] = pData[1];
      cmdCompleteEvt[5] = pData[2];
      cmdCompleteEvt[6] = 0x00;
      USBSendData( cmdCompleteEvt, 7 );
    }
    RingMemDelete( &usbRingParm, dataLen+4 );
 }
}

/*******************************************************************************
 * @fn      PRBS9_Get
 *
 * @brief   数据转换为PRBS9类型
 *
 * @param   pData - 待处理数据
 * @param   len   - 数据长度
 *
 * @return  None.
 */
uint16_t PRBS9_INIT=0x01FF;
__HIGH_CODE
void PRBS9_Get( uint8_t *pData, uint16_t len )
{
  uint8_t outData = 0;
  uint8_t i,j;
  for(j = 0; j < len; j++)
  {
    outData=0;
    for( i = 0; i < 8; i++)
    {
      outData |= (PRBS9_INIT&0x01) << i;
      PRBS9_INIT = (PRBS9_INIT>>1)|(((PRBS9_INIT^(PRBS9_INIT>>4))&0x0001)<<8);
    }
    pData[j] = outData;
  }
}

/*******************************************************************************
 * @fn      PRBS15_Get
 *
 * @brief   数据转换为PRBS15类型
 *
 * @param   pData - 待处理数据
 * @param   len   - 数据长度
 *
 * @return  None.
 */
uint16_t PRBS15_INIT=0x7FFF;
__HIGH_CODE
void PRBS15_Get( uint8_t *pData, uint16_t len )
{
  uint8_t outData = 0;
  uint8_t i,j;
  for(j = 0; j < len; j++)
  {
    outData = 0;
    for( i = 0; i < 8; i++)
    {
      outData |= (PRBS15_INIT&0x01) << i;
      PRBS15_INIT = (PRBS15_INIT>>1)|(((PRBS15_INIT^(PRBS15_INIT>>1))&0x0001)<<14);
    }
    pData[j] = outData;
  }
}

/*******************************************************************************
 * @fn      TX_DATA
 *
 * @brief   数据转化为指定类型
 *
 * @param   buf - 待处理数据
 * @param   len - 数据长度
 *
 * @return  None.
 */
__HIGH_CODE
void TX_DATA( uint8_t *buf, uint8_t len )
{
  switch( buf[0])
  {
    case 0:
    {
      PRBS9_Get( &buf[2], len );
      break;
    }
    case 1:
    {
        __wrap_memset(&buf[2],0xF0, len+4 );
      break;
    }
    case 2:
    {

        __wrap_memset(&buf[2],0xAA, len+4 );
      break;
    }
    case 3:
    {
        PRBS15_Get( &buf[2], len );
        break;
    }
    case 4:
    {
        __wrap_memset(&buf[2],0xFF, len+4 );
      break;
    }
    case 5:
    {
        __wrap_memset(&buf[2],0x00, len+4 );
      break;
    }
    case 6:
    {
        __wrap_memset(&buf[2],0x0F, len+4 );
      break;
    }
    case 7:
    {
        __wrap_memset(&buf[2],0x55, len+4 );
      break;
    }
  }
  buf[1] = len;
}

/*******************************************************************************
 * @fn      DtmProcess
 *
 * @brief   dtm
 *
 * @param   None.
 *
 * @return  None.
 */
__HIGH_CODE
void DtmProcess(void)
{
    UART_Process_Data();
    USB_Process_Data();
    if(ttflag)
    {
      mDelayuS(1);
      ttflag = 0;
      TX_DATA( TxBuf, TxBuf[1] );
      rf_tx_start(TxBuf);
    }
}

/******************************** endfile @ main ******************************/
