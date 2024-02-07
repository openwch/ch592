/********************************** (C) COPYRIGHT *******************************
* File Name          : pdfFile.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        :
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __PDF_FILE_H
#define __PDF_FILE_H

#include "CH59x_common.h"

#define  PDF_TMP_BUF_LEN_MAX   512
#define  PDF_TMP_BUF_LEN_EXT   256
//#define  PDF_SAVE_ADDRESS      ((uint32_t)(128*1024))
#define  PDF_FILE_MAX_LEN      (2*1024*1024)

uint32_t pdf_data_proces( void *buf, uint32_t length );

void open_file( char *filename );

void write_file( uint8_t *pData );

void close_file( void );

void pdf_memcpy( void *pDst, const void *pSrc, uint32_t len );
#endif

