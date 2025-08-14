/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_gpio.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

/*********************************************************************
 * @fn      GPIOA_ModeCfg
 *
 * @brief   GPIOA�˿�����ģʽ����
 *
 * @param   pin     - PA4-PA15,��ӦоƬ12��GPIO����
 * @param   mode    - �����������
 *
 * @return  none
 */
void GPIOA_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)
{
    switch(mode)
    {
        case GPIO_ModeIN_Floating:
            R32_PA_PD_DRV &= ~pin;
            R32_PA_PU &= ~pin;
            R32_PA_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PU:
            R32_PA_PD_DRV &= ~pin;
            R32_PA_PU |= pin;
            R32_PA_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PD:
            R32_PA_PD_DRV |= pin;
            R32_PA_PU &= ~pin;
            R32_PA_DIR &= ~pin;
            break;

        case GPIO_ModeOut_PP_5mA:
            R32_PA_PD_DRV &= ~pin;
            R32_PA_DIR |= pin;
            break;

        case GPIO_ModeOut_PP_20mA:
            R32_PA_PD_DRV |= pin;
            R32_PA_DIR |= pin;
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      GPIOB_ModeCfg
 *
 * @brief   GPIOB�˿�����ģʽ����
 *
 * @param   pin     - PB0,PB4,PB6-PB7,PB10-PB15,PB22-PB23,��ӦоƬ12��GPIO����
 * @param   mode    - �����������
 *
 * @return  none
 */
void GPIOB_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)
{
    switch(mode)
    {
        case GPIO_ModeIN_Floating:
            R32_PB_PD_DRV &= ~pin;
            R32_PB_PU &= ~pin;
            R32_PB_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PU:
            R32_PB_PD_DRV &= ~pin;
            R32_PB_PU |= pin;
            R32_PB_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PD:
            R32_PB_PD_DRV |= pin;
            R32_PB_PU &= ~pin;
            R32_PB_DIR &= ~pin;
            break;

        case GPIO_ModeOut_PP_5mA:
            R32_PB_PD_DRV &= ~pin;
            R32_PB_DIR |= pin;
            break;

        case GPIO_ModeOut_PP_20mA:
            R32_PB_PD_DRV |= pin;
            R32_PB_DIR |= pin;
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      GPIOA_ITModeCfg
 *
 * @brief   GPIOA�����ж�ģʽ����
 *
 * @param   pin     - PA4-PA15,��ӦоƬ12��GPIO����
 * @param   mode    - ��������
 *
 * @return  none
 */
void GPIOA_ITModeCfg(uint32_t pin, GPIOITModeTpDef mode)
{
    switch(mode)
    {
        case GPIO_ITMode_LowLevel: // �͵�ƽ����
            R16_PA_INT_MODE &= ~pin;
            R32_PA_CLR |= pin;
            break;

        case GPIO_ITMode_HighLevel: // �ߵ�ƽ����
            R16_PA_INT_MODE &= ~pin;
            R32_PA_OUT |= pin;
            break;

        case GPIO_ITMode_FallEdge: // �½��ش���
            R16_PA_INT_MODE |= pin;
            R32_PA_CLR |= pin;
            break;

        case GPIO_ITMode_RiseEdge: // �����ش���
            R16_PA_INT_MODE |= pin;
            R32_PA_OUT |= pin;
            break;

        default:
            break;
    }
    R16_PA_INT_IF = pin;
    R16_PA_INT_EN |= pin;
}

/*********************************************************************
 * @fn      GPIOB_ITModeCfg
 *
 * @brief   GPIOB�����ж�ģʽ����
 *
 * @param   pin     - PB0,PB4,PB6-PB7,PB10-PB15,PB22-PB23,��ӦоƬ12��GPIO����
 * @param   mode    - ��������
 *
 * @return  none
 */
void GPIOB_ITModeCfg(uint32_t pin, GPIOITModeTpDef mode)
{
    uint32_t Pin = pin | ((pin & (GPIO_Pin_22 | GPIO_Pin_23)) >> 14);
    switch(mode)
    {
        case GPIO_ITMode_LowLevel: // �͵�ƽ����
            R16_PB_INT_MODE &= ~Pin;
            R32_PB_CLR |= pin;
            break;

        case GPIO_ITMode_HighLevel: // �ߵ�ƽ����
            R16_PB_INT_MODE &= ~Pin;
            R32_PB_OUT |= pin;
            break;

        case GPIO_ITMode_FallEdge: // �½��ش���
            R16_PB_INT_MODE |= Pin;
            R32_PB_CLR |= pin;
            break;

        case GPIO_ITMode_RiseEdge: // �����ش���
            R16_PB_INT_MODE |= Pin;
            R32_PB_OUT |= pin;
            break;

        default:
            break;
    }
    R16_PB_INT_IF = Pin;
    R16_PB_INT_EN |= Pin;
}

/*********************************************************************
 * @fn      GPIOPinRemap
 *
 * @brief   ���蹦������ӳ��
 *
 * @param   s       - �Ƿ�ʹ��ӳ��
 * @param   perph   - RB_RF_ANT_SW_EN -  RF antenna switch control output on PA4/PA5/PA12/PA13/PA14/PA15
 *                    RB_PIN_MODEM  -  MODEM: PA6/PA7 -> PB12/PB13
 *                    RB_PIN_PWMX   -  PWMX: PA12/PA13 -> PA6/PA7
 *                    RB_PIN_SPI0   -  SPI0:  PA12/PA13/PA14/PA15 -> PB12/PB13/PB14/PB15
 *                    RB_PIN_UART2  -  UART2: PB22/PB23 ->  PA6/PA7
 *                    RB_PIN_UART1  -  UART1: PA8/PA9 ->  PB12/PB13
 *                    RB_PIN_UART0  -  UART0: PB4/PB7 ->  PA15/PA14
 *                    RB_PIN_TMR2   -  TMR2:  PA11 ->  PB11
 *                    RB_PIN_TMR1   -  TMR1:  PA10 ->  PB10
 *                    RB_PIN_TMR0   -  TMR0:  PA9 ->  PB23
 *
 * @return  none
 */
void GPIOPinRemap(FunctionalState s, uint16_t perph)
{
    if(s)
    {
        R16_PIN_ALTERNATE |= perph;
    }
    else
    {
        R16_PIN_ALTERNATE &= ~perph;
    }
}

/*********************************************************************
 * @fn      GPIOAGPPCfg
 *
 * @brief   ģ������GPIO���Ź��ܿ���
 *
 * @param   s       -   ENABLE  - ��ģ�����蹦�ܣ��ر����ֹ���
 *                      DISABLE - �������ֹ��ܣ��ر�ģ�����蹦��
 * @param   perph   -   RB_PIN_USB_DP_PU  - USB UD+�����ڲ���������
 *                      RB_PIN_USB_IE     - USB ����
 *
 * @return  none
 */
void GPIOAGPPCfg(FunctionalState s, uint16_t perph)
{
    if(s)
    {
        R16_PIN_ANALOG_IE |= perph;
    }
    else
    {
        R16_PIN_ANALOG_IE &= ~perph;
    }
}
