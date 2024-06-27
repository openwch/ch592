/********************************** (C) COPYRIGHT *******************************
* File Name          : peripheral.h
* Author             : Hikari
* Version            : V1.0
* Date               : 2024/01/18
* Description        : 
*******************************************************************************/

#ifndef __peripheral_H
#define __peripheral_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "CH59x_common.h"
#include <PAW3395.h>
#include "mouse.h"
#include "nvs_flash.h"
#include <EC10.h>

#define MODE_USB            0
#define MODE_2_4G           1
#define MODE_BT             2

extern uint8_t work_mode;
void peripheral_init(void);
void peripheral_process(void);
void peripheral_pairing_cb(void);
void peripheral_connecting_cb(void);
void peripheral_disconnected_cb(void);;
void peripheral_connected_cb(void);
void peripheral_enter_sleep(void);
void peripheral_exit_sleep(void);
void peripheral_sleep_update(void);
void peripheral_pilot_led_receive(uint8_t led);

#ifdef __cplusplus
}
#endif

#endif
