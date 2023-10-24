/********************************** (C) COPYRIGHT *******************************
* File Name          : usb_connect.c
* Author             : WCH
* Version            : V1.0
* Date               : 2014/05/12
* Description        :
*******************************************************************************/

/******************************************************************************/
#include "usb_connect.h"

//#define CONFIG_USB_CONNECT_DEBUG
#ifdef CONFIG_USB_CONNECT_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/


/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/

/**************************************************************************************************
 * @fn      HAL_USBConnCheckInit
 *
 * @brief   Initialization detects whether USB is connected
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_USBConnCheckInit(void)
{
    GPIOA_ModeCfg(USB_CHECK_BV, GPIO_ModeIN_PD);
    GPIOA_ITModeCfg( USB_CHECK_BV, GPIO_ITMode_RiseEdge );            // Rising edge trigger
    PFIC_EnableIRQ( GPIO_A_IRQn );
}

/******************************** endfile @ usb_connect ******************************/
