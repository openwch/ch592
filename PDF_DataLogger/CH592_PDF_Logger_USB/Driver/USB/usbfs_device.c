/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_usbotg_device.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2022/08/20
* Description        : This file provides all the USBOTG firmware functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#include "usbfs_device.h"
#include "SW_UDISK.h"
#include "usb_desc.h"
#include "pdfFile.h"

#define USB_REQ_FEAT_REMOTE_WAKEUP  0x01
#define USB_REQ_FEAT_ENDP_HALT      0x00

#define DEF_STRING_DESC_LANG        0x00
#define DEF_STRING_DESC_MANU        0x01
#define DEF_STRING_DESC_PROD        0x02
#define DEF_STRING_DESC_SERN        0x03


/*******************************************************************************/
/* Variable Definition */
/* Global */
const    uint8_t  *pUSBFS_Descr;

/* Setup Request */
volatile uint8_t  USBFS_SetupReqCode;
volatile uint8_t  USBFS_SetupReqType;
volatile uint16_t USBFS_SetupReqValue;
volatile uint16_t USBFS_SetupReqIndex;
volatile uint16_t USBFS_SetupReqLen;

/* USB Device Status */
volatile uint8_t  USBFS_DevConfig;
volatile uint8_t  USBFS_DevAddr;
volatile uint8_t  USBFS_DevSleepStatus;
volatile uint8_t  USBFS_DevEnumStatus;

/* Endpoint Buffer */
__attribute__ ((aligned(4))) uint8_t USBFS_EP0_Buf[ DEF_USBD_UEP0_SIZE ];    //ep0(64)
__attribute__ ((aligned(4))) uint8_t UDisk_In_Buf[ DEF_UDISK_PACK_64 ];
__attribute__ ((aligned(4))) uint8_t UDisk_Out_Buf[ DEF_UDISK_PACK_64 ];

/* USB IN Endpoint Busy Flag */
volatile uint8_t  USBFS_Endp_Busy[ DEF_UEP_NUM ];

__attribute__((aligned(4))) uint8_t RxBuffer[MAX_PACKET_SIZE]; // IN, must even address
__attribute__((aligned(4))) uint8_t TxBuffer[MAX_PACKET_SIZE]; // OUT, must even address


/*********************************************************************
 * @fn      USB_DeviceInit
 *
 * @brief   USB设备功能初始化，4个端点，8个通道。
 *
 * @param   none
 *
 * @return  none
 */
void USBFS_DeviceInit(void)
{
    R8_USB_CTRL = 0x00;

    R8_UEP2_3_MOD = RB_UEP2_TX_EN | RB_UEP3_RX_EN; // endpoint2 OUT, endpoint3 IN

    R16_UEP0_DMA = (uint16_t)(uint32_t)pEP0_RAM_Addr;
    R16_UEP2_DMA = (uint16_t)(uint32_t)pEP2_RAM_Addr;
    R16_UEP3_DMA = (uint16_t)(uint32_t)pEP3_RAM_Addr;

    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    R8_UEP2_CTRL = UEP_T_RES_NAK;
    R8_UEP3_CTRL = UEP_R_RES_ACK;

    R8_USB_DEV_AD = 0x00;
    R8_USB_CTRL = RB_UC_DEV_PU_EN | RB_UC_INT_BUSY | RB_UC_DMA_EN; // 启动USB设备及DMA，在中断期间中断标志未清除前自动返回NAK
    R16_PIN_ANALOG_IE |= RB_PIN_USB_IE | RB_PIN_USB_DP_PU;         // 防止USB端口浮空及上拉电阻
    R8_USB_INT_FG = 0xFF;                                          // 清中断标志
    R8_UDEV_CTRL = RB_UD_PD_DIS | RB_UD_PORT_EN;                   // 允许USB端口
    R8_USB_INT_EN = RB_UIE_SUSPEND | RB_UIE_BUS_RST | RB_UIE_TRANSFER;
}


/*********************************************************************
 * @fn      USBFS_Device_Init
 *
 * @brief   Initializes USB device.
 *
 * @return  none
 */
void USBFS_Device_Init( void )
{
    uint8_t i;

    pEP0_RAM_Addr = USBFS_EP0_Buf;
    pEP2_RAM_Addr = UDisk_In_Buf;
    pEP3_RAM_Addr = UDisk_Out_Buf;

    /* Clear End-points Busy Status */
    for( i=0; i<DEF_UEP_NUM; i++ )
    {
        USBFS_Endp_Busy[ i ] = 0;
    }

    USBFS_DeviceInit();
    PFIC_EnableIRQ(USB_IRQn);
}


/*********************************************************************
 * @fn      USBFS_Endp_DataUp
 *
 * @brief   USBFS device data upload
 *
 * @return  none
 */
uint8_t USBFS_Endp_DataUp(uint8_t endp, uint8_t *pbuf, uint16_t len, uint8_t mod)
{
    uint8_t endp_mode;
    uint8_t buf_load_offset;

    /* DMA config, endp_ctrl config, endp_len config */
    if( (endp>=DEF_UEP1) && (endp<=DEF_UEP7) )
    {
        if( USBFS_Endp_Busy[ endp ] == 0 )
        {
            if( (endp == DEF_UEP1) || (endp == DEF_UEP4) )
            {
                /* endp1/endp4 */
                endp_mode = USBFSD_UEP_MOD(0);
                if( endp == DEF_UEP1 )
                {
                    endp_mode = (uint8_t)(endp_mode>>4);
                }
            }
            else if( (endp == DEF_UEP2) || (endp == DEF_UEP3) )
            {
                /* endp2/endp3 */
                endp_mode = USBFSD_UEP_MOD(1);
                if( endp == DEF_UEP3 )
                {
                    endp_mode = (uint8_t)(endp_mode>>4);
                }
            }
            else if( (endp == DEF_UEP5) || (endp == DEF_UEP6) )
            {
                /* endp5/endp6 */
                endp_mode = USBFSD_UEP_MOD(2);
                if( endp == DEF_UEP6 )
                {
                    endp_mode = (uint8_t)(endp_mode>>4);
                }
            }
            else
            {
                /* endp7 */
                endp_mode = USBFSD_UEP_MOD(3);
            }

            if( endp_mode & USBFSD_UEP_TX_EN )
            {
                if( endp_mode & USBFSD_UEP_RX_EN )
                {
                    buf_load_offset = 64;
                }
                else
                {
                    buf_load_offset = 0;
                }

                if( buf_load_offset == 0 )
                {
                    if( mod == DEF_UEP_DMA_LOAD )
                    {
                        /* DMA mode */
                        USBFSD_UEP_DMA(endp) = (uint16_t)(uint32_t)pbuf;
                    }
                    else
                    {
                        /* copy mode */
                        pdf_memcpy( USBFSD_UEP_BUF(endp), pbuf, len );
                    }
                }
                else
                {
                    pdf_memcpy( USBFSD_UEP_BUF(endp)+buf_load_offset, pbuf, len );
                }
                /* Set end-point busy */
                USBFS_Endp_Busy[ endp ] = 0x01;
                /* tx length */
                USBFSD_UEP_TLEN(endp) = len;
                /* response ack */
                USBFSD_UEP_CTRL(endp) = (USBFSD_UEP_CTRL(endp) & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }

    return 0;
}


/*********************************************************************
 * @fn      USB_DevTransProcess
 *
 * @brief   USB 传输处理函数
 *
 * @return  none
 */
void USB_DevTransProcess(void)
{
    uint16_t len;
    uint8_t intflag, errflag;

    intflag = R8_USB_INT_FG;

    if(intflag & RB_UIF_TRANSFER)
    {
        if((R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN) // 非空闲
        {
            switch(R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
            // 分析操作令牌和端点号
            {
                /* end-point 0 data in interrupt */
                case UIS_TOKEN_IN | DEF_UEP0:
                {
                    switch(USBFS_SetupReqCode)
                    {
                        case USB_GET_DESCRIPTOR:
                            len = USBFS_SetupReqLen >= DEF_USBD_UEP0_SIZE ? DEF_USBD_UEP0_SIZE : USBFS_SetupReqLen; // 本次传输长度
                            pdf_memcpy(pEP0_DataBuf, pUSBFS_Descr, len);                          /* 加载上传数据 */
                            USBFS_SetupReqLen -= len;
                            pUSBFS_Descr += len;
                            R8_UEP0_T_LEN = len;
                            R8_UEP0_CTRL ^= RB_UEP_T_TOG; // 翻转
                            break;

                        case USB_SET_ADDRESS:
                            R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | USBFS_DevAddr;
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            break;

                        default:
                            R8_UEP0_T_LEN = 0; // 状态阶段完成中断或者是强制上传0长度数据包结束控制传输
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            break;
                    }
                }
                break;

                case UIS_TOKEN_OUT | DEF_UEP0:
                    len = R8_USB_RX_LEN;
                    break;

                case UIS_TOKEN_IN | DEF_UEP2:
                    R8_UEP2_CTRL ^= RB_UEP_T_TOG;
                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    USBFS_Endp_Busy[ DEF_UEP2 ] = 0;
                    UDISK_In_EP_Deal();
                    break;

                case UIS_TOKEN_OUT | DEF_UEP3:
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)
                    { // 不同步的数据包将丢弃
                        R8_UEP3_CTRL ^= RB_UEP_R_TOG;
                        len = R8_USB_RX_LEN;
                        R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_R_RES) | UEP_R_RES_NAK;
                        UDISK_Out_EP_Deal(UDisk_Out_Buf,len);
                        R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_R_RES) | UEP_R_RES_ACK;
                    }
                break;

                default:
                    break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }

        if(R8_USB_INT_ST & RB_UIS_SETUP_ACT) // Setup包处理
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            /* Store All Setup Values */
            USBFS_SetupReqType  = pSetupReqPak->bRequestType;
            USBFS_SetupReqCode  = pSetupReqPak->bRequest;
            USBFS_SetupReqLen   = pSetupReqPak->wLength;
            USBFS_SetupReqValue = pSetupReqPak->wValue;
            USBFS_SetupReqIndex = pSetupReqPak->wIndex;

            len = 0;
            errflag = 0;
            if((USBFS_SetupReqType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
            {
                /* usb non-standard request processing */
                if (( USBFS_SetupReqType & USB_REQ_TYP_MASK ) == USB_REQ_TYP_CLASS)
                {
                    if (USBFS_SetupReqCode == CMD_UDISK_GET_MAX_LUN)
                    {
                        USBFS_EP0_Buf[0] = 0;
                        pUSBFS_Descr = (uint8_t*)USBFS_EP0_Buf;
                        len = 1;
                    }
                    else if (USBFS_SetupReqCode == CMD_UDISK_RESET)
                    {
                        /* UDisk Reset */
                        Udisk_Sense_Key = 0x00;
                        Udisk_Sense_ASC = 0x00;
                        Udisk_CSW_Status = 0x00;
                        Udisk_Transfer_Status = 0x00;
                        UDISK_Transfer_DataLen = 0x00;
                    }
                    else
                    {
                        errflag = 0xFF;
                    }
                }
                else
                {
                    errflag = 0xFF; /* 非标准请求 */
                }
            }
            else /* 标准请求 */
            {
                switch(USBFS_SetupReqCode)
                {
                    case USB_GET_DESCRIPTOR:
                    {
                        switch((uint8_t)( USBFS_SetupReqValue >> 8 ))
                        {
                            /* get usb device descriptor */
                            case USB_DESCR_TYP_DEVICE:
                            {
                                pUSBFS_Descr = MyDevDescr;
                                len = DEF_USBD_DEVICE_DESC_LEN;
                            }
                            break;

                            /* get usb configuration descriptor */
                            case USB_DESCR_TYP_CONFIG:
                            {
                                pUSBFS_Descr = MyCfgDescr;
                                len = DEF_USBD_CONFIG_DESC_LEN;
                            }
                            break;

                            case USB_DESCR_TYP_STRING:
                            {
                                switch( (uint8_t)( USBFS_SetupReqValue & 0xFF ) )
                                {
                                    /* Descriptor 0, Language descriptor */
                                    case DEF_STRING_DESC_LANG:
                                        pUSBFS_Descr = MyLangDescr;
                                        len = DEF_USBD_LANG_DESC_LEN;
                                        break;

                                    /* Descriptor 1, Manufacturers String descriptor */
                                    case DEF_STRING_DESC_MANU:
                                        pUSBFS_Descr = MyManuInfo;
                                        len = DEF_USBD_MANU_DESC_LEN;
                                        break;

                                    /* Descriptor 2, Product String descriptor */
                                    case DEF_STRING_DESC_PROD:
                                        pUSBFS_Descr = MyProdInfo;
                                        len = DEF_USBD_PROD_DESC_LEN;
                                        break;

                                    /* Descriptor 3, Serial-number String descriptor */
                                    case DEF_STRING_DESC_SERN:
                                        pUSBFS_Descr = MySerNumInfo;
                                        len = DEF_USBD_SN_DESC_LEN;
                                        break;

                                    default:
                                        errflag = 0xFF; // 不支持的字符串描述符
                                        break;
                                }
                            }
                            break;

                            default:
                                errflag = 0xff;
                                break;
                        }
                        if(USBFS_SetupReqLen > len)
                            USBFS_SetupReqLen = len; //实际需上传总长度
                        len = (USBFS_SetupReqLen >= DEF_USBD_UEP0_SIZE) ? DEF_USBD_UEP0_SIZE : USBFS_SetupReqLen;
                        pdf_memcpy(pEP0_DataBuf, pUSBFS_Descr, len);
                        pUSBFS_Descr += len;
                    }
                    break;

                    case USB_SET_ADDRESS:
                        USBFS_DevAddr = (uint8_t)( USBFS_SetupReqValue & 0xFF );
                        break;

                    case USB_GET_CONFIGURATION:
                        pEP0_DataBuf[0] = USBFS_DevConfig;
                        if(USBFS_SetupReqLen > 1)
                            USBFS_SetupReqLen = 1;
                        break;

                    case USB_SET_CONFIGURATION:
                        USBFS_DevConfig = (uint8_t)( USBFS_SetupReqValue & 0xFF );
                        USBFS_DevEnumStatus = 0x01;
                        break;

                    case USB_CLEAR_FEATURE:
                    {
                        if((USBFS_SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // 端点
                        {
                            switch(USBFS_SetupReqIndex & 0xff)
                            {
                                case 0x82:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x02:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x81:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x01:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                default:
                                    errflag = 0xFF; // 不支持的端点
                                    break;
                            }
                        }
                        else
                            errflag = 0xFF;
                    }
                    break;

                    case USB_GET_INTERFACE:
                        pEP0_DataBuf[0] = 0x00;
                        if(USBFS_SetupReqLen > 1)
                            USBFS_SetupReqLen = 1;
                        break;

                    case USB_GET_STATUS:
                        pEP0_DataBuf[0] = 0x00;
                        pEP0_DataBuf[1] = 0x00;
                        if(USBFS_SetupReqLen > 2)
                            USBFS_SetupReqLen = 2;
                        break;

                    default:
                        errflag = 0xff;
                        break;
                }
            }

            if(errflag == 0xff) // 错误或不支持
            {
                //                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
            }
            else
            {
                if(USBFS_SetupReqType & 0x80) // 上传
                {
                    len = (USBFS_SetupReqLen > DEF_USBD_UEP0_SIZE) ? DEF_USBD_UEP0_SIZE : USBFS_SetupReqLen;
                    USBFS_SetupReqLen -= len;
                }
                else
                {
                    len = 0; // 下传
                }
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; // 默认数据包是DATA1
            }

            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
    }
    else if(intflag & RB_UIF_BUS_RST)
    {
        R8_USB_DEV_AD = 0;
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_USB_INT_FG = RB_UIF_BUS_RST;
    }
    else if(intflag & RB_UIF_SUSPEND)
    {
        if(R8_USB_MIS_ST & RB_UMS_SUSPEND)
        {
            ;
        } // 挂起
        else
        {
            ;
        } // 唤醒
        R8_USB_INT_FG = RB_UIF_SUSPEND;
    }
    else
    {
        R8_USB_INT_FG = intflag;
    }
}


/*********************************************************************
 * @fn      USB_IRQHandler
 *
 * @brief   USB中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void USB_IRQHandler(void) /* USB中断服务程序,使用寄存器组1 */
{
    USB_DevTransProcess();
}
