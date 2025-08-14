/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/01/25
 * Description        : ģ�����HID�豸
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

#define DevEP0SIZE    0x40
#define DevEP1SIZE    0x40
// �豸������
const uint8_t MyDevDescr[] = {0x12,0x01,0x10,0x01,0x00,0x00,0x00,DevEP0SIZE,0x3d,0x41,0x07,0x21,0x00,0x00,0x00,0x00,0x00,0x01};
// ����������
const uint8_t MyCfgDescr[] = {
    0x09,0x02,0x29,0x00,0x01,0x01,0x04,0xA0,0x23,               //����������
    0x09,0x04,0x00,0x00,0x02,0x03,0x00,0x00,0x05,               //�ӿ�������
    0x09,0x21,0x00,0x01,0x00,0x01,0x22,0x22,0x00,               //HID��������
    0x07,0x05,0x81,0x03,DevEP1SIZE,0x00,0x01,              //�˵�������
    0x07,0x05,0x01,0x03,DevEP1SIZE,0x00,0x01               //�˵�������
};
/*�ַ�����������*/
/*HID�౨��������*/
const uint8_t HIDDescr[] = {  0x06, 0x00,0xff,
                              0x09, 0x01,
                              0xa1, 0x01,                                                   //���Ͽ�ʼ
                              0x09, 0x02,                                                   //Usage Page  �÷�
                              0x15, 0x00,                                                   //Logical  Minimun
                              0x26, 0x00,0xff,                                              //Logical  Maximun
                              0x75, 0x08,                                                   //Report Size
                              0x95, 0x40,                                                   //Report Counet
                              0x81, 0x06,                                                   //Input
                              0x09, 0x02,                                                   //Usage Page  �÷�
                              0x15, 0x00,                                                   //Logical  Minimun
                              0x26, 0x00,0xff,                                              //Logical  Maximun
                              0x75, 0x08,                                                   //Report Size
                              0x95, 0x40,                                                   //Report Counet
                              0x91, 0x06,                                                   //Output
                              0xC0};

/**********************************************************/
uint8_t        DevConfig, Ready = 0;
uint8_t        SetupReqCode;
uint16_t       SetupReqLen;
const uint8_t *pDescr;
uint8_t        Report_Value = 0x00;
uint8_t        Idle_Value = 0x00;
uint8_t        USB_SleepStatus = 0x00; /* USB˯��״̬ */

//HID�豸�жϴ������ϴ�������4�ֽڵ�����
uint8_t HID_Buf[DevEP1SIZE] = {0,0,0,0};

/******** �û��Զ������˵�RAM ****************************************/
__attribute__((aligned(4))) uint8_t EP0_Databuf[64 + 64 + 64]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4))) uint8_t EP1_Databuf[64 + 64];      //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4))) uint8_t EP2_Databuf[64 + 64];      //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4))) uint8_t EP3_Databuf[64 + 64];      //ep3_out(64)+ep3_in(64)

/*********************************************************************
 * @fn      USB_DevTransProcess
 *
 * @brief   USB ���䴦����
 *
 * @return  none
 */
void USB_DevTransProcess(void)  //USB�豸�����жϴ���
{
    uint8_t len, chtype;        //len���ڿ���������chtype���ڴ�����ݴ��䷽����������͡����յĶ������Ϣ
    uint8_t intflag, errflag = 0;   //intflag���ڴ�ű�־�Ĵ���ֵ��errflag���ڱ���Ƿ�֧��������ָ��

    intflag = R8_USB_INT_FG;        //ȡ���жϱ�ʶ�Ĵ�����ֵ

    if(intflag & RB_UIF_TRANSFER)   //�ж�_INT_FG�е�USB��������жϱ�־λ�����д�������ж��ˣ���if���
    {
        if((R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN) // �ǿ���   //�ж��ж�״̬�Ĵ����е�5:4λ���鿴���Ƶ�PID��ʶ��������λ����11����ʾ���У�����if���
        {
            switch(R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))    //ȡ�����Ƶ�PID��ʶ���豸ģʽ�µ�3:0λ�Ķ˵�š�����ģʽ��3:0λ��Ӧ��PID��ʶλ
            // �����������ƺͶ˵��
            {                           //�˵�0���ڿ��ƴ��䡣���µĶ˵�0��IN��OUT������Ӧ���򣬶�Ӧ���ƴ�������ݽ׶κ�״̬�׶Ρ�
                case UIS_TOKEN_IN:      //���ư���PIDΪIN��5:4λΪ10��3:0λ�Ķ˵��Ϊ0��IN���ƣ��豸�����������ݡ�_UIS_��USB�ж�״̬
                {                       //�˵�0Ϊ˫��˵㣬�������ƴ��䡣 ��|0������ʡ����
                    switch(SetupReqCode)//���ֵ�����յ�SETUP��ʱ��ֵ���ں������SETUP��������򣬶�Ӧ���ƴ�������ý׶Ρ�
                    {
                        case USB_GET_DESCRIPTOR:    //USB��׼���������USB�豸��ȡ����
                            len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen; // ���ΰ����䳤�ȡ��Ϊ64�ֽڣ�����64�ֽڵķֶ�δ���ǰ����Ҫ������
                            memcpy(pEP0_DataBuf, pDescr, len);//memcpy:�ڴ濽����������(����λ)��ַ����(����λ)�ַ������ȵ�(һ��λ)��ַ��
                            //DMAֱ�����ڴ����������⵽�ڴ�ĸ�д�������õ�Ƭ�����ƾͿ��Խ��ڴ��е����ݷ��ͳ�ȥ�����ֻ���������黥�ำֵ�����漰��DMAƥ��������ڴ棬���޷�����DMA��
                            SetupReqLen -= len;     //��¼ʣ�µ���Ҫ���͵����ݳ���
                            pDescr += len;          //���½�������Ҫ���͵����ݵ���ʼ��ַ,����������
                            R8_UEP0_T_LEN = len;    //�˵�0���ͳ��ȼĴ�����д�뱾�ΰ����䳤��
                            R8_UEP0_CTRL ^= RB_UEP_T_TOG;   // ͬ���л���IN���򣨶��ڵ�Ƭ������T���򣩵�PID�е�DATA0��DATA1�л�
                            break;                  //��ֵ��˵���ƼĴ��������ְ���Ӧ��ACK��NAK��STALL������Ӳ������ɷ��Ϲ淶�İ���DMA�Զ�����
                        case USB_SET_ADDRESS:       //USB��׼�������Ϊ�豸����һ��Ψһ��ַ����Χ0��127��0ΪĬ�ϵ�ַ
                            R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                                    //7λ��ַ+���λ���û��Զ����ַ��Ĭ��Ϊ1�������ϡ������䳤�ȡ�������ġ������䳤�ȡ��ں��渳ֵ���˵�ַλ��
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                                    //R��ӦOUT����ACK��T��ӦIN����NAK�����CASE��֧����IN���򣬵�DMA��Ӧ�ڴ��У���Ƭ��û�����ݸ���ʱ����NAK���ְ���
                            break;                                                  //һ��������OUT�����豸��ذ�������������ӦNAK��

                        case USB_SET_FEATURE:       //USB��׼�������Ҫ������һ�����豸���ӿڻ�˵��ϵ�����
                            break;

                        default:
                            R8_UEP0_T_LEN = 0;      //״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ��䣨�����ֶγ���Ϊ0�����ݰ�������SYNC��PID��EOP�ֶζ��У�
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                                    //R��ӦOUT����ACK��T��ӦIN����NAK�����CASE��֧����OUT���򣬵�DMA��Ӧ�ڴ��и����������ҵ�Ƭ����������ʱ����ACK���ְ���
                            Ready = 1;
                            PRINT("Ready_STATUS = %d\n",Ready);
                            break;
                    }
                }
                break;

                case UIS_TOKEN_OUT:     //���ư���PIDΪOUT��5:4λΪ00��3:0λ�Ķ˵��Ϊ0��OUT���ƣ��������豸�����ݡ�
                {                       //�˵�0Ϊ˫��˵㣬�������ƴ��䡣 ��|0������ʡ����
                    len = R8_USB_RX_LEN;    //��ȡ��ǰUSB���ճ��ȼĴ����д洢�Ľ��յ������ֽ��� //���ճ��ȼĴ���Ϊ�����˵㹲�ã����ͳ��ȼĴ������и���
                }
                break;

                case UIS_TOKEN_OUT | 1: //���ư���PIDΪOUT���˵��Ϊ1
                {
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)   //Ӳ�����ж��Ƿ���ȷ��ͬ���л����ݰ���ͬ���л���ȷ����һλ�Զ���λ
                    { // ��ͬ�������ݰ�������
                        R8_UEP1_CTRL ^= RB_UEP_R_TOG;   //OUT�����DATAͬ���л����趨һ������ֵ��
                        len = R8_USB_RX_LEN;        //��ȡ�������ݵ��ֽ���
                        DevEP1_OUT_Deal(len);       //���ͳ���Ϊlen���ֽڣ��Զ���ACK���ְ����Զ���ĳ���
                    }
                }
                break;

                case UIS_TOKEN_IN | 1: //���ư���PIDΪIN���˵��Ϊ1
                    R8_UEP1_CTRL ^= RB_UEP_T_TOG;       //IN�����DATA�л�һ�¡��趨��Ҫ���͵İ���PID��
                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;    //��DMA��û���ɵ�Ƭ����������ʱ����T��ӦIN������ΪNAK�������˾ͷ������ݡ�
                    Ready = 1;
                    PRINT("Ready_IN_EP1 = %d\n",Ready);
                    break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;    //д1�����жϱ�־
        }

        if(R8_USB_INT_ST & RB_UIS_SETUP_ACT) // Setup������
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
                        //R��ӦOUT�����ڴ�ΪDATA1��DMA�յ������ݰ���PIDҪΪDATA1�����������ݴ���Ҫ�ش�����ACK��DMA��Ӧ�ڴ����յ������ݣ���Ƭ������������
                        //T��ӦIN�����趨ΪDATA1����Ƭ������������DMA��Ӧ�ڴ棬��DATA1���ͳ�ȥ����NAK����Ƭ��û��׼�������ݣ���
            SetupReqLen = pSetupReqPak->wLength;    //���ݽ׶ε��ֽ���      //pSetupReqPak�����˵�0��RAM��ַǿ��ת����һ����Žṹ��ĵ�ַ���ṹ���Ա���ν�������
            SetupReqCode = pSetupReqPak->bRequest;  //��������
            chtype = pSetupReqPak->bRequestType;    //�������ݴ��䷽����������͡����յĶ������Ϣ

            len = 0;
            errflag = 0;
            if((pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) //�ж���������ͣ�������Ǳ�׼���󣬽�if���
            {
                /* �Ǳ�׼���� */
                /* ��������,�������󣬲�������� */
                if(pSetupReqPak->bRequestType & 0x40)   //ȡ�������е�ĳһλ���ж��Ƿ�Ϊ0����Ϊ���if���
                {
                    /* �������� */
                }
                else if(pSetupReqPak->bRequestType & 0x20)  //ȡ�������е�ĳһλ���ж��Ƿ�Ϊ0����Ϊ���if���
                {   //�ж�ΪHID������
                    switch(SetupReqCode)    //�ж���������
                    {
                        case DEF_USB_SET_IDLE: /* 0x0A: SET_IDLE */         //����������HID�豸�ض����뱨��Ŀ���ʱ����
                            Idle_Value = EP0_Databuf[3];
                            break; //���һ��Ҫ��

                        case DEF_USB_SET_REPORT: /* 0x09: SET_REPORT */     //����������HID�豸�ı���������
                            break;

                        case DEF_USB_SET_PROTOCOL: /* 0x0B: SET_PROTOCOL */ //����������HID�豸��ǰ��ʹ�õ�Э��
                            Report_Value = EP0_Databuf[2];
                            break;

                        case DEF_USB_GET_IDLE: /* 0x02: GET_IDLE */         //�������ȡHID�豸�ض����뱨��ĵ�ǰ�Ŀ��б���
                            EP0_Databuf[0] = Idle_Value;
                            len = 1;
                            break;

                        case DEF_USB_GET_PROTOCOL: /* 0x03: GET_PROTOCOL */     //��������HID�豸��ǰ��ʹ�õ�Э��
                            EP0_Databuf[0] = Report_Value;
                            len = 1;
                            break;

                        default:
                            errflag = 0xFF;
                    }
                }
            }
            else    //�ж�Ϊ��׼����
            {
                switch(SetupReqCode)    //�ж���������
                {
                    case USB_GET_DESCRIPTOR:    //�������ñ�׼������
                    {
                        switch(((pSetupReqPak->wValue) >> 8))   //����8λ����ԭ���ĸ�8λ�Ƿ�Ϊ0��Ϊ1��ʾ����ΪIN�������s-case���
                        {
                            case USB_DESCR_TYP_DEVICE:  //��ͬ��ֵ����ͬ��������������豸������
                            {
                                pDescr = MyDevDescr;    //���豸�������ַ�������pDescr��ַ�У�����ñ�׼�����������caseĩβ���ÿ�����������
                                len = MyDevDescr[0];    //Э��涨�豸�����������ֽڴ���ֽ������ȡ������������õ�len����
                            }
                            break;

                            case USB_DESCR_TYP_CONFIG:  //������������������
                            {
                                pDescr = MyCfgDescr;    //�������������ַ�������pDescr��ַ�У�֮��ᷢ��
                                len = MyCfgDescr[2];    //Э��涨�����������ĵ������ֽڴ��������Ϣ���ܳ�
                            }
                            break;

                            case USB_DESCR_TYP_HID:     //���������˻��ӿ������������˴��ṹ���е�wIndex��������������ͬ����Ϊ�ӿںš�
                                switch((pSetupReqPak->wIndex) & 0xff)       //ȡ�Ͱ�λ���߰�λĨȥ
                                {
                                    /* ѡ��ӿ� */
                                    case 0:
                                        pDescr = (uint8_t *)(&MyCfgDescr[18]);  //�ӿ�1�������������λ�ã�������
                                        len = 9;
                                        break;

                                    default:
                                        /* ��֧�ֵ��ַ��������� */
                                        errflag = 0xff;
                                        break;
                                }
                                break;

                            case USB_DESCR_TYP_REPORT:  //���������豸����������
                            {
                                if(((pSetupReqPak->wIndex) & 0xff) == 0) //�ӿ�0����������
                                {
                                    pDescr = HIDDescr; //����׼���ϴ�
                                    len = sizeof(HIDDescr);
                                }
                                else
                                    len = 0xff; //������ֻ��2���ӿڣ���仰����������ִ��
                            }
                            break;

                            case USB_DESCR_TYP_STRING:  //���������豸�ַ���������
                            {
                                switch((pSetupReqPak->wValue) & 0xff)   //����wValue��ֵ�����ַ�����Ϣ
                                {
                                    default:
                                        errflag = 0xFF; // ��֧�ֵ��ַ���������
                                        break;
                                }
                            }
                            break;

                            default:
                                errflag = 0xff;
                                break;
                        }
                        if(SetupReqLen > len)
                            SetupReqLen = len;      //ʵ�����ϴ��ܳ���
                        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;   //��󳤶�Ϊ64�ֽ�
                        memcpy(pEP0_DataBuf, pDescr, len);  //��������
                        pDescr += len;
                    }
                    break;

                    case USB_SET_ADDRESS:       //�����������豸��ַ
                        SetupReqLen = (pSetupReqPak->wValue) & 0xff;    //�������ַ���λ�豸��ַ�ݴ���SetupReqLen��
                        break;                                          //���ƽ׶λḳֵ���豸��ַ����

                    case USB_GET_CONFIGURATION: //���������豸��ǰ����
                        pEP0_DataBuf[0] = DevConfig;    //���豸���÷Ž�RAM
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;    //�����ݽ׶ε��ֽ�����1����ΪDevConfigֻ��һ���ֽ�
                        break;

                    case USB_SET_CONFIGURATION: //�����������豸��ǰ����
                        DevConfig = (pSetupReqPak->wValue) & 0xff;  //ȡ�Ͱ�λ���߰�λĨȥ
                        break;

                    case USB_CLEAR_FEATURE:     //�ر�USB�豸������/���ܡ��������豸���Ƕ˵�����ϵġ�
                    {
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) //�ж��ǲ��Ƕ˵�����������˵�ֹͣ������״̬��
                        {
                            switch((pSetupReqPak->wIndex) & 0xff)   //ȡ�Ͱ�λ���߰�λĨȥ���ж�����
                            {       //16λ�����λ�ж����ݴ��䷽��0ΪOUT��1ΪIN����λΪ�˵�š�
                                case 0x81:      //����_TOG��_T_RES����λ����������д��_NAK����ӦIN����NAK��ʾ�����ݷ���
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x01:      //����_TOG��_R_RES����λ����������д��_ACK����ӦOUT����ACK��ʾ��������
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                default:
                                    errflag = 0xFF; // ��֧�ֵĶ˵�
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)  //�ж��ǲ����豸�����������豸���ѣ�
                        {
                            if(pSetupReqPak->wValue == 1)   //���ѱ�־λΪ1
                            {
                                USB_SleepStatus &= ~0x01;   //���λ����
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                    }
                    break;

                    case USB_SET_FEATURE:       //����USB�豸������/���ܡ��������豸���Ƕ˵�����ϵġ�
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) //�ж��ǲ��Ƕ˵�������ʹ�˵�ֹͣ������
                        {
                            /* �˵� */
                            switch(pSetupReqPak->wIndex)    //�ж�����
                            {       //16λ�����λ�ж����ݴ��䷽��0ΪOUT��1ΪIN����λΪ�˵�š�
                                case 0x81:      //����_TOG��_T_RES��λ����������д��_STALL����������ָ��ֹͣ�˵�Ĺ���
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x01:      //����_TOG��_R_RES��λ����������д��_STALL����������ָ��ֹͣ�˵�Ĺ���
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                default:
                                    /* ��֧�ֵĶ˵� */
                                    errflag = 0xFF; // ��֧�ֵĶ˵�
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)  //�ж��ǲ����豸������ʹ�豸���ߣ�
                        {
                            if(pSetupReqPak->wValue == 1)
                            {
                                USB_SleepStatus |= 0x01;    //����˯��
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                        break;

                    case USB_GET_INTERFACE:     //�������ýӿڵ�ǰ������ѡ������ֵ
                        pEP0_DataBuf[0] = 0x00;
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;    //�����ݽ׶ε��ֽ�����1����Ϊ��������ֻ��һ���ֽ�
                        break;

                    case USB_SET_INTERFACE:     //�����뼤���豸��ĳ���ӿ�
                        break;

                    case USB_GET_STATUS:        //���������豸���ӿڻ��Ƕ˵��״̬
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) //�ж��Ƿ�Ϊ�˵�״̬
                        {
                            /* �˵� */
                            pEP0_DataBuf[0] = 0x00;
                            switch(pSetupReqPak->wIndex)
                            {       //16λ�����λ�ж����ݴ��䷽��0ΪOUT��1ΪIN����λΪ�˵�š�
                                case 0x81:      //�ж�_TOG��_T_RES��λ��������STALL״̬����if���
                                    if((R8_UEP1_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01; //����D0Ϊ1����ʾ�˵㱻ֹͣ�����ˡ���λ��SET_FEATURE��CLEAR_FEATURE�������á�
                                    }
                                    break;

                                case 0x01:      //�ж�_TOG��_R_RES��λ��������STALL״̬����if���
                                    if((R8_UEP1_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)  //�ж��Ƿ�Ϊ�豸״̬
                        {
                            pEP0_DataBuf[0] = 0x00;
                            if(USB_SleepStatus)     //����豸����˯��״̬
                            {
                                pEP0_DataBuf[0] = 0x02;     //���λD0Ϊ0����ʾ�豸�����߹��磬Ϊ1��ʾ�豸�Թ��硣 D1λΪ1��ʾ֧��Զ�̻��ѣ�Ϊ0��ʾ��֧�֡�
                            }
                            else
                            {
                                pEP0_DataBuf[0] = 0x00;
                            }
                        }
                        pEP0_DataBuf[1] = 0;    //����״̬��Ϣ�ĸ�ʽΪ16λ�����߰�λ����Ϊ0
                        if(SetupReqLen >= 2)
                        {
                            SetupReqLen = 2;    //�����ݽ׶ε��ֽ�����2����Ϊ��������ֻ��2���ֽ�
                        }
                        break;

                    default:
                        errflag = 0xff;
                        break;
                }
            }
            if(errflag == 0xff) // �����֧��
            {
                //                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
                Ready = 1;
                PRINT("Ready_Stall = %d\n",Ready);
            }
            else
            {
                if(chtype & 0x80)   // �ϴ������λΪ1�����ݴ��䷽��Ϊ�豸���������䡣
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                    len = 0;        // �´������λΪ0�����ݴ��䷽��Ϊ�������豸���䡣
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;     // Ĭ�����ݰ���DATA1
            }

            R8_USB_INT_FG = RB_UIF_TRANSFER;    //д1���жϱ�ʶ
        }
    }


    else if(intflag & RB_UIF_BUS_RST)   //�ж�_INT_FG�е����߸�λ��־λ��Ϊ1����
    {
        R8_USB_DEV_AD = 0;      //�豸��ַд��0�����������·�����豸һ���µ�ַ
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;   //�Ѷ˵�0�Ŀ��ƼĴ�����д�ɣ�������Ӧ��ӦACK��ʾ�����յ���������ӦNAK��ʾû������Ҫ����
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_USB_INT_FG = RB_UIF_BUS_RST; //д1���жϱ�ʶ
    }
    else if(intflag & RB_UIF_SUSPEND)   //�ж�_INT_FG�е����߹�������¼��жϱ�־λ������ͻ��Ѷ��ᴥ�����ж�
    {
        if(R8_USB_MIS_ST & RB_UMS_SUSPEND)  //ȡ������״̬�Ĵ����еĹ���״̬λ��Ϊ1��ʾUSB���ߴ��ڹ���̬��Ϊ0��ʾ���ߴ��ڷǹ���̬
        {
            Ready = 0;
            PRINT("Ready_Sleep = %d\n",Ready);
        } // ����     //���豸���ڿ���״̬����3ms��������Ҫ���豸���������ڵ������ߣ�
        else    //��������жϱ���������û�б��ж�Ϊ����
        {
            Ready = 1;
            PRINT("Ready_WeakUp = %d\n",Ready);
        } // ����
        R8_USB_INT_FG = RB_UIF_SUSPEND; //д1���жϱ�־
    }
    else
    {
        R8_USB_INT_FG = intflag;    //_INT_FG��û���жϱ�ʶ���ٰ�ԭֵд��ԭ���ļĴ���
    }
}

/*********************************************************************
 * @fn      DevHIDReport
 *
 * @brief   �ϱ�HID����
 *
 * @return  0���ɹ�
 *          1������
 */
void DevHIDReport(uint8_t data0,uint8_t data1,uint8_t data2,uint8_t data3)
{
    HID_Buf[0] = data0;
    HID_Buf[1] = data1;
    HID_Buf[2] = data2;
    HID_Buf[3] = data3;
    memcpy(pEP1_IN_DataBuf, HID_Buf, sizeof(HID_Buf));
    DevEP1_IN_Deal(DevEP1SIZE);
}

/*********************************************************************
 * @fn      DevWakeup
 *
 * @brief   �豸ģʽ��������
 *
 * @return  none
 */
void DevWakeup(void)
{
    R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_DP_PU);
    R8_UDEV_CTRL |= RB_UD_LOW_SPEED;
    mDelaymS(2);
    R8_UDEV_CTRL &= ~RB_UD_LOW_SPEED;
    R16_PIN_ANALOG_IE |= RB_PIN_USB_DP_PU;
}

/*********************************************************************
 * @fn      DebugInit
 *
 * @brief   ���Գ�ʼ��
 *
 * @return  none
 */
void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 */
int main()
{
    uint8_t s;
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    DebugInit();        //���ô���1����prinft��debug
    printf("start\n");

    pEP0_RAM_Addr = EP0_Databuf;    //���û�����64�ֽڡ�
    pEP1_RAM_Addr = EP1_Databuf;

    USB_DeviceInit();

    PFIC_EnableIRQ(USB_IRQn);       //�����ж�����
    mDelaymS(100);

    while(1)
    {//ģ�⴫��4���ֽڵ����ݣ�ʵ�ʴ�������û���Ҫ�����޸�
        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x05, 0x10, 0x20, 0x11);
        }
        mDelaymS(100);

        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x0A, 0x15, 0x25, 0x22);
        }
        mDelaymS(100);

        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x0E, 0x1A, 0x2A, 0x44);
        }
        mDelaymS(100);

        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x10, 0x1E, 0x2E, 0x88);
        }
        mDelaymS(100);
    }
}

/*********************************************************************
 * @fn      DevEP1_OUT_Deal
 *
 * @brief   �˵�1���ݴ����յ����ݺ�ȡ���ٷ���ȥ���û����и��ġ�
 *
 * @return  none
 */
void DevEP1_OUT_Deal(uint8_t l)
{ /* �û����Զ��� */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP1_IN_DataBuf[i] = ~pEP1_OUT_DataBuf[i];
    }
    DevEP1_IN_Deal(l);
}


/*********************************************************************
 * @fn      USB_IRQHandler
 *
 * @brief   USB�жϺ���
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void USB_IRQHandler(void) /* USB�жϷ������,ʹ�üĴ�����1 */
{
    USB_DevTransProcess();
}
