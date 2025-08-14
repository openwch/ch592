/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_mesh_example.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2025/04/27
 * Description        : mesh传输程序例子
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef _LWNS_MESH_EXAMPLE_H_
#define _LWNS_MESH_EXAMPLE_H_

#include "lwns_config.h"

#define MESH_EXAMPLE_TX_PERIOD_EVT    1 << (0)
#define MESH_EXAMPLE_TX_NODE_EVT      1 << (1)
void lwns_mesh_process_init(void);

#endif /* _LWNS_MESH_EXAMPLE_H_ */
