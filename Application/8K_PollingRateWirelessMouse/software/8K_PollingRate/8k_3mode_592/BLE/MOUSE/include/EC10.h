/********************************** (C) COPYRIGHT *******************************
* File Name          : EC10.h
* Author             : Hikari
* Version            : V1.0
* Date               : 2024/01/18
* Description        : 
*******************************************************************************/

#ifndef __EC10_H
#define __EC10_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "CH59x_common.h"


#define EC_A             (GPIO_Pin_11)       //A
#define EC_B             (GPIO_Pin_10)       //A


#define  EC_A_ST         GPIOA_ReadPortPin(EC_A)
#define  EC_B_ST         GPIOA_ReadPortPin(EC_B)

void EC10_init(void);
void EC10_process(void);
uint8_t EC10_scan(void);
void EC10_press(void);
void EC10_release(void);

#ifdef __cplusplus
}
#endif

#endif
