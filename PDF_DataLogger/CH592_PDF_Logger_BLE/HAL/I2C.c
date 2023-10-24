/********************************** (C) COPYRIGHT *******************************
 * File Name          : I2C.c
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
/* Include */
#include "HAL.h"


/**************************************************************************************************
 * @fn      HAL_I2C_Init
 *
 * @brief   I2C bus initialization
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_I2C_Init(void)
{
    I2C_SCL_HIGH(); //set SCL high
    I2C_SDA_HIGH(); //set SDA high
    GPIOA_ModeCfg(GPIO_Pin_6 | GPIO_Pin_7, GPIO_ModeOut_PP_5mA); //i2c gpio init
}

/**************************************************************************************************
 * @fn      HAL_I2C_Start
 *
 * @brief   I2C bus send start
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_I2C_Start(void)
{
    SDA_OUT();
    I2C_SCL_HIGH();
    I2C_SDA_HIGH();
    DelayUs(4);
    I2C_SDA_LOW();
    DelayUs(4);
    I2C_SCL_LOW();
}

/**************************************************************************************************
 * @fn      HAL_I2C_Stop
 *
 * @brief   I2C bus send stop
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_I2C_Stop(void)
{
    SDA_OUT();
    I2C_SCL_LOW();
    I2C_SDA_LOW();
    DelayUs(4);
    I2C_SCL_HIGH();
    DelayUs(4);
    I2C_SDA_HIGH();
    DelayUs(4);
}

/**************************************************************************************************
 * @fn      HAL_I2C_WaitAck
 *
 * @brief   I2C wait ACK
 *
 * @param   None
 *
 * @return  0 - ACK received
 *          1 - ACK not received
 **************************************************************************************************/
uint8_t HAL_I2C_WaitAck(void)
{
    UINT16 waittime = 0;

    SDA_IN();
    I2C_SDA_HIGH();
    DelayUs(1);
    I2C_SCL_HIGH();
    DelayUs(1);
    while (READ_SDA() != 0)
    {
        ++waittime;
        if (waittime > 250)
        {
            HAL_I2C_Stop();

            return 1;
        }
    }
    I2C_SCL_LOW();

    return 0;
}

/**************************************************************************************************
 * @fn      HAL_I2C_Ack
 *
 * @brief   I2C bus send ACK
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_I2C_Ack(void)
{
    I2C_SCL_LOW();
    SDA_OUT();
    I2C_SDA_LOW();
    DelayUs(2);
    I2C_SCL_HIGH();
    DelayUs(2);
    I2C_SCL_LOW();
}

/**************************************************************************************************
 * @fn      HAL_I2C_NAck
 *
 * @brief   I2C bus send NACK
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_I2C_NAck(void)
{
    I2C_SCL_LOW();
    SDA_OUT();
    I2C_SDA_HIGH();
    DelayUs(2);
    I2C_SCL_HIGH();
    DelayUs(2);
    I2C_SCL_LOW();
}

/**************************************************************************************************
 * @fn      HAL_I2C_WriteByte
 *
 * @brief   I2C bus sends a byte
 *
 * @param   txd - Bytes to be sent
 *
 * @return  None
 **************************************************************************************************/
void HAL_I2C_WriteByte(uint8_t txd)
{
    uint8_t i;

    SDA_OUT();
    I2C_SCL_LOW();

    for (i = 0; i < 8; ++i)
    {
        if (txd & 0x80)
            I2C_SDA_HIGH();
        else
            I2C_SDA_LOW();

        txd <<= 1;
        DelayUs(2);
        I2C_SCL_HIGH();
        DelayUs(2);
        I2C_SCL_LOW();
        DelayUs(2);
    }
}

/**************************************************************************************************
 * @fn      HAL_I2C_ReadByte
 *
 * @brief   I2C receives a byte
 *
 * @param   ack_en - Reply ACK or NACK after receiving data, 0 - NACK, 1 - ACK
 *
 * @return  Byte received
 **************************************************************************************************/
uint8_t HAL_I2C_ReadByte(uint8_t ack_en)
{
    uint8_t i, rxd = 0;

    SDA_IN();
    for (i = 0; i < 8; ++i)
    {
        I2C_SCL_LOW();
        DelayUs(2);
        I2C_SCL_HIGH();
        rxd <<= 1;
        if (READ_SDA() != 0)
            ++rxd;
        DelayUs(1);
    }

    if (ack_en != 0)
        HAL_I2C_Ack();
    else
        HAL_I2C_NAck();

    return rxd;
}

/**************************************************************************************************
 * @fn      HAL_I2C_WriteCmd
 *
 * @brief   I2C send command
 *
 * @param   addr - the I2C device write address
 *          p_buf - points to a data buffer
 *          len - the length of the data buffer
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_I2C_WriteCmd(uint8_t addr, uint8_t *p_buf, UINT16 len)
{
    UINT16 i;

    /* send a start */
    HAL_I2C_Start();

    /* send the write addr */
    HAL_I2C_WriteByte(addr<<1 | 0x00);
    if(HAL_I2C_WaitAck() != 0)
    {
        HAL_I2C_Stop();

        return 1;
    }

    /* write the data */
    for(i = 0; i < len; ++i)
    {
        HAL_I2C_WriteByte(p_buf[i]);
        if(HAL_I2C_WaitAck() != 0)
        {
            HAL_I2C_Stop();

            return 1;
        }
    }

    /* send a stop */
    HAL_I2C_Stop();

    return 0;
}

/**************************************************************************************************
 * @fn      HAL_I2C_WriteReg
 *
 * @brief   I2C send register
 *
 * @param   addr - the I2C device write address
 *          reg - the iic register address
 *          p_buf - points to a data buffer
 *          len - the length of the data buffer
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t *p_buf, UINT16 len)
{
    UINT16 i;

    /* send a start */
    HAL_I2C_Start();

    /* send the write addr */
    HAL_I2C_WriteByte(addr<<1 | 0x00);
    if(HAL_I2C_WaitAck() != 0)
    {
        HAL_I2C_Stop();

        return 1;
    }

    /* send the reg */
    HAL_I2C_WriteByte(reg);
    if(HAL_I2C_WaitAck() != 0)
    {
        HAL_I2C_Stop();

        return 1;
    }

    /* write the data */
    for(i = 0; i < len; ++i)
    {
        HAL_I2C_WriteByte(p_buf[i]);
        if(HAL_I2C_WaitAck() != 0)
        {
            HAL_I2C_Stop();

            return 1;
        }
    }

    /* send a stop */
    HAL_I2C_Stop();

    return 0;
}

/**************************************************************************************************
 * @fn      HAL_I2C_ReadCmd
 *
 * @brief   I2C receive command
 *
 * @param   addr - the I2C device write address
 *          p_buf - points to a data buffer
 *          len - the length of the data buffer
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_I2C_ReadCmd(uint8_t addr, uint8_t *p_buf, UINT16 len)
{
    /* send a start */
    HAL_I2C_Start();

    /* send the read addr */
    HAL_I2C_WriteByte((addr<<1) | 0x01);
    if(HAL_I2C_WaitAck() != 0)
    {
        HAL_I2C_Stop();

        return 1;
    }

    /* read the data */
    while(len != 0)
    {
        /* if the last */
        if(len == 1)
        {
            /* send nack */
            *p_buf = HAL_I2C_ReadByte(0);
        }
        else
        {
            /* send ack */
            *p_buf = HAL_I2C_ReadByte(1);
        }
        --len;
        ++p_buf;
    }

    /* send a stop */
    HAL_I2C_Stop();

    return 0;
}

/**************************************************************************************************
 * @fn      HAL_I2C_ReadReg
 *
 * @brief   I2C read register
 *
 * @param   addr - the I2C device write address
 *          reg - the iic register address
 *          p_buf - points to a data buffer
 *          len - the length of the data buffer
 *
 * @return  0 - SUCCESS, 1 - FAILURE
 **************************************************************************************************/
uint8_t HAL_I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *p_buf, UINT16 len)
{
    /* send a start */
    HAL_I2C_Start();

    /* send the write addr */
    HAL_I2C_WriteByte(addr<<1 | 0x00);
    if(HAL_I2C_WaitAck() != 0)
    {
        HAL_I2C_Stop();

        return 1;
    }

    /* send the reg */
    HAL_I2C_WriteByte(reg);
    if(HAL_I2C_WaitAck() != 0)
    {
        HAL_I2C_Stop();

        return 1;
    }

    /* send a start */
    HAL_I2C_Start();

    /* send the read addr */
    HAL_I2C_WriteByte((addr<<1) | 0x01);
    if(HAL_I2C_WaitAck() != 0)
    {
        HAL_I2C_Stop();

        return 1;
    }

    /* read the data */
    while(len != 0)
    {
        /* if the last */
        if(len == 1)
        {
            /* send nack */
            *p_buf = HAL_I2C_ReadByte(0);
        }
        else
        {
            /* send ack */
            *p_buf = HAL_I2C_ReadByte(1);
        }
        --len;
        ++p_buf;
    }

    /* send a stop */
    HAL_I2C_Stop();

    return 0;
}

/******************************** endfile @ I2C ******************************/
