/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_broadcast_example.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2025/04/27
 * Description        : broadcast，广播程序例子
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef _LWNS_BROADCAST_EXAMPLE_H_
#define _LWNS_BROADCAST_EXAMPLE_H_

#include "lwns_config.h"

#define BROADCAST_EXAMPLE_TX_PERIOD_EVT    1 << (0)

void lwns_broadcast_process_init(void);

#endif /* LWNS_BROADCAST_EXAMPLE_H_ */
