/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_sec.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2025/04/27
 * Description        : lwns加密数据处理接口
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef _LWNS_SEC_H_
#define _LWNS_SEC_H_

#include "lwns_config.h"

int lwns_msg_encrypt(uint8_t *src, uint8_t *to, uint8_t mlen);

int lwns_msg_decrypt(uint8_t *src, uint8_t *to, uint8_t mlen);

#endif /* _LWNS_SEC_H_ */
