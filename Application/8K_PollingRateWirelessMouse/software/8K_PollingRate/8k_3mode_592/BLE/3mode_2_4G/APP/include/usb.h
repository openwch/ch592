/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/11/12
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef USB_H
#define USB_H

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_USB_DEBUG        0


#define DevEP0SIZE    0x08
#define DevEP1SIZE    0x08
#define DevEP2SIZE    0x10
#define DevEP3SIZE    0x20
#define DevEP6SIZE    0x40
#define USB_WBVAL(x) ((x) & 0xFF),(((x) >> 8) & 0xFF)

#define REPORT_ID_KEYBOARD          0x01
#define REPORT_ID_ALL_KEYBOARD      0x02
#define REPORT_ID_CONSUMER          0x03
#define REPORT_ID_SYS_CTL           0x04

#define         HOST_SET_FEATURE    (1<<0)
#define         HOST_SET_SUSPEND    (1<<1)
#define         HOST_WAKEUP_ENABLE  (1<<2)

extern volatile uint8_t USB_SleepStatus;
extern volatile uint8_t    USB_READY_FLAG;
extern uint8_t  usb_enum_success_flag;

extern uint8_t  led_val;

typedef void (*USB_receive_cb_t)(uint8_t *pData,uint8_t len);

extern void USB_Init( void );

extern void USB_Uinit( void );

extern void USB_Wake_up( void );

extern void USB_receive_cb_register(USB_receive_cb_t cback);

extern uint8_t USB_class_keyboard_report(uint8_t *pData,uint8_t len);

extern uint8_t USB_mouse_report(uint8_t *pData,uint8_t len);

extern uint8_t USB_all_keyboard_report(uint8_t *pData,uint8_t len);

extern uint8_t USB_consumer_report(uint8_t *pData,uint8_t len);

extern uint8_t USB_sys_ctl_report(uint8_t data);

extern uint8_t USB_manufacturer_report(uint8_t *pData,uint8_t len);

extern uint8_t USB_IAP_report(uint8_t *pData,uint8_t len);

extern void USB_cfg_vid_pid( uint16_t VID, uint16_t PID );

extern void USB_cfg_serial_num( uint8_t *pData, uint8_t len );
extern void USB_cfg_prod_info( uint8_t *pData, uint8_t len );
extern void USB_cfg_manu_info( uint8_t *pData, uint8_t len );


#ifdef __cplusplus
}
#endif

#endif
