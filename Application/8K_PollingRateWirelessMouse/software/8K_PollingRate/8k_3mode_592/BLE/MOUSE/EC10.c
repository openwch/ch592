/********************************** (C) COPYRIGHT *******************************
 * File Name          : EC10.c
 * Author             : Hikari
 * Version            : V1.0
 * Date               : 2024/01/18
 * Description        :
 *
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "peripheral.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * @fn      EC10_init
 *
 * @brief   EC10_init
 *
 * @return  null
 */
void EC10_init()
{
    GPIOA_ModeCfg(EC_A|EC_B ,GPIO_ModeIN_PU);
}

/*********************************************************************
 * @fn      EC10_scan
 *
 * @brief   EC10_scan
 *
 * @return  null
 */
__HIGH_CODE
uint8_t EC10_scan()
{
#if (0)//20-20
    uint8_t new_A_st;
    static uint8_t first_A_st=0;
    static uint8_t last_A_st=0;
    new_A_st = EC_A_ST;
    PRINT("EC %x %x\n",EC_A_ST,EC_B_ST);
    if(first_A_st && (!last_A_st) && (!new_A_st))
    {
        if(EC_B_ST)
        {
            //正转
            PRINT("EC +\n");
            sys_data_byte[1] = 0xE9;
            sys_data_byte[2] = 0x00;
            trans_send_data(sys_data_byte,2+1);
        }
        else
        {
            //反转
            PRINT("EC -\n");
            sys_data_byte[1] = 0xEA;
            sys_data_byte[2] = 0x00;
            trans_send_data(sys_data_byte,2+1);
        }
    }
    first_A_st = last_A_st;
    last_A_st = new_A_st;
#else   // 30-15
    static uint8_t last_A_st=0;
    static uint8_t last_B_st=0;
    static uint8_t first_A_st=0;
    static uint8_t first_B_st=0;
    uint8_t new_A_st;
    uint8_t new_B_st;
    uint8_t ret=0;

    R32_PA_PD_DRV &= ~(EC_A|EC_B);
    R32_PA_PU |= (EC_A|EC_B);
    R32_PA_DIR &= ~(EC_A|EC_B);

    new_A_st = !EC_A_ST;
    new_B_st = !EC_B_ST;
//    PRINT("EC %x %x \n",new_A_st,new_B_st);
    if((last_A_st==first_A_st) && (last_B_st==first_B_st) && ((new_A_st!=last_A_st)||(new_B_st!=last_B_st)) && (new_A_st==new_B_st))
    {
        peripheral_sleep_update();
        if(new_A_st == last_A_st)
        {
            //正转
//            PRINT("EC +\n");
            ret = 0x01;
        }
        else {
            //反转
//            PRINT("EC -\n");
            ret = 0xFF;
        }
    }
    first_A_st = last_A_st;
    first_B_st = last_B_st;
    last_A_st = new_A_st;
    last_B_st = new_B_st;
    return ret;
#endif
}


/******************************** endfile @rf ******************************/
