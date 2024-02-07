/********************************** (C) COPYRIGHT *******************************
 * File Name          : I2C.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/09/01
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#ifndef __I2C_H_
#define __I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
#define SDA_OUT()           (GPIOA_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA))    //SDA set as output
#define SDA_IN()            (GPIOA_ModeCfg(GPIO_Pin_7, GPIO_ModeIN_PU))         //SDA set as input
#define READ_SDA()          (GPIOA_ReadPortPin(GPIO_Pin_7))                     //read SDA data

#define I2C_SCL_HIGH()      (GPIOA_SetBits(GPIO_Pin_6))     //Set SCL high
#define I2C_SCL_LOW()       (GPIOA_ResetBits(GPIO_Pin_6))   //Set SCL low
#define I2C_SDA_HIGH()      (GPIOA_SetBits(GPIO_Pin_7))     //Set SDA high
#define I2C_SDA_LOW()       (GPIOA_ResetBits(GPIO_Pin_7))   //Set SDA low


/*********************************************************************
 * FUNCTIONS
 */

/**
 * @brief   I2C bus initialization
 */
void HAL_I2C_Init(void);

/**
 * @brief   I2C bus send start
 */
void HAL_I2C_Start(void);

/**
 * @brief   I2C bus send stop
 */
void HAL_I2C_Stop(void);

/**
 * @brief   HAL_I2C_WaitAck
 */
uint8_t HAL_I2C_WaitAck(void);

/**
 * @brief   I2C bus send ACK
 */
void HAL_I2C_Ack(void);

/**
 * @brief   I2C bus send NACK
 */
void HAL_I2C_NAck(void);

/**
 * @brief   I2C bus sends a byte
 */
void HAL_I2C_WriteByte(uint8_t txd);

/**
 * @brief   I2C receives a byte
 */
uint8_t HAL_I2C_ReadByte(uint8_t ack_en);

/**
 * @brief   I2C send command
 */
uint8_t HAL_I2C_WriteCmd(uint8_t addr, uint8_t *p_buf, UINT16 len);

/**
 * @brief   I2C send register
 */
uint8_t HAL_I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t *p_buf, UINT16 len);

/**
 * @brief   I2C receive command
 */
uint8_t HAL_I2C_ReadCmd(uint8_t addr, uint8_t *p_buf, UINT16 len);

/**
 * @brief   I2C read register
 */
uint8_t HAL_I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *p_buf, UINT16 len);

/********************************************************************************
********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H_ */
