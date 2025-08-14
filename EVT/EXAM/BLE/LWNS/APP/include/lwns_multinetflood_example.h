/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_multinetflood_example.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2025/04/27
 * Description        : multinetflood，组播网络泛洪传输例子
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef _LWNS_MULTINETFLOOD_EXAMPLE_H_
#define _LWNS_MULTINETFLOOD_EXAMPLE_H_

#include "lwns_config.h"

#define MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT    1 << (0)

void lwns_multinetflood_process_init(void);

#endif /* _LWNS_MULTINETFLOOD_EXAMPLE_H_ */
