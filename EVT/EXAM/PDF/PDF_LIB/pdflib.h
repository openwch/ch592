/********************************** (C) COPYRIGHT *******************************
* File Name          : pdf.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        :
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __PDFLIB_H__
#define __PDFLIB_H__



/*  native OS integer types */
typedef  signed int          LPDF_INT;
typedef  unsigned int        LPDF_UINT;

/*  32bit integer types
 */
typedef  signed int          LPDF_INT32;
typedef  unsigned int        LPDF_UINT32;


/*  16bit integer types
 */
typedef  signed short        LPDF_INT16;
typedef  unsigned short      LPDF_UINT16;


/*  8bit integer types
 */
typedef  signed char         LPDF_INT8;
typedef  unsigned char       LPDF_UINT8;


/*  8bit binary types
 */
typedef  unsigned char       LPDF_BYTE;


/*  float type (32bit IEEE754)
 */
typedef  float               LPDF_REAL;


/*  boolean type (0: False, !0: True)
 */
typedef  signed int          LPDF_BOOL;

/*  error-no type (32bit unsigned integer)
 */
typedef  unsigned long       LPDF_STATUS;


typedef uint32_t (*pFnPdfProcessData)( void *pBuf,uint32_t len );


#define LPDF_OK                             0
#define LPDF_FAILED                         1
#define LPDF_TRUE                           1
#define LPDF_FALSE                          0
#define LPDF_NOERROR                        0
#define LPDF_INVALID_OBJECT                 0x1033
#define LPDF_INVALID_IMAGE                  0x1030
#define LPDF_INVALID_COLOR_SPACE            0x1020
#define LPDF_INVALID_IMAGE_WIDTH            0x1031
#define LPDF_PAGE_INVALID_SIZE				0x1054
#define LPDF_INVALID_DOCUMENT               0x1025
#define LPDF_PAGE_INVALID_FONT              0x104F

/*----- font state -----------------------------------------------------------*/
#define   LPDF_FONT_INVALID                 0x00FF
#define   LPDF_FONT_COURIER                 0x0000
#define   LPDF_FONT_COURIER_B               0x0001
#define   LPDF_FONT_COURIER_O               0x0002
#define   LPDF_FONT_COURIER_OB              0x0003
#define   LPDF_FONT_HELVETICA               0x0004
#define   LPDF_FONT_HELVETICA_B             0x0005
#define   LPDF_FONT_HELVETICA_O             0x0006
#define   LPDF_FONT_HELVETICA_BO            0x0007
#define   LPDF_FONT_TIMES_R                 0x0008
#define   LPDF_FONT_TIMES_B                 0x0009
#define   LPDF_FONT_TIMES_I                 0x000A
#define   LPDF_FONT_TIMES_BI                0x000B
#define   LPDF_FONT_SYMBOL                  0x000C
#define   LPDF_FONT_ZAP                     0x000D
/*----- Graphis mode ---------------------------------------------------------*/
#define   LPDF_GMODE_PAGE_DESCRIPTION       0x0001
#define   LPDF_GMODE_PATH_OBJECT            0x0002
#define   LPDF_GMODE_TEXT_OBJECT            0x0004
#define   LPDF_GMODE_CLIPPING_PATH          0x0008
#define   LPDF_GMODE_SHADING                0x0010
#define   LPDF_GMODE_INLINE_IMAGE           0x0020
#define   LPDF_GMODE_EXTERNAL_OBJECT        0x0040
#define   LPDF_GMODE_INVALID                0x0080
#define   LPDF_GMODE_OVER                   0x0100

#define OBJ_MAX_NUM     125

typedef struct
{
    pFnPdfProcessData pdfProcessCb;
    LPDF_INT8 *pPdfBuf;
}LPDF_Init;

typedef enum 
{
    LPDF_OBJ_INFO = 0,
    LPDF_OBJ_STREAM,
    LPDF_OBJ_FONT,
    LPDF_OBJ_PAGE,
    LPDF_OBJ_CATALOG,
    LPDF_OBJ_PAGES,
    LPDF_OBJ_IMAGE,
    LPDF_OBJ_EOF
}LPDF_ObjType;

typedef struct
{
	LPDF_ObjType type;
	LPDF_UINT32 offset;
	LPDF_UINT16 number;
	LPDF_UINT32 attr[10];
	void *doc;
}LPDF_Obj;

typedef struct
{
	LPDF_Obj obj[OBJ_MAX_NUM];
	LPDF_UINT32 offset;
	LPDF_UINT16 pagesNumber;
	LPDF_UINT16 objNumber;
	LPDF_UINT16 imageNumber;
	LPDF_STATUS error;
}LPDF_Doc_Rec;

typedef LPDF_Obj *LPDF_Page;
typedef LPDF_Doc_Rec *LPDF_Doc;


/*----- LPDF Interfaces-------------------------------------------------------*/
LPDF_STATUS LPDF_InitParam( LPDF_Init *pParam );

LPDF_Doc LPDF_New( LPDF_Doc pdf );

LPDF_STATUS LPDF_SaveToFile(LPDF_Doc pdf);

LPDF_STATUS LPDF_Page_SetFontAndSize(LPDF_Page page,const char *font_name, LPDF_UINT8 size);

LPDF_STATUS LPDF_Page_Get_Font_Text_Width(LPDF_Page page, const char* font_name,
    const char* text, LPDF_UINT8 size, LPDF_REAL* text_width);

LPDF_STATUS LPDF_Page_SetWordSpace(LPDF_Page page, LPDF_REAL  value);

LPDF_UINT16 LPDF_Page_GetHeight(LPDF_Page page);

LPDF_UINT16 LPDF_Page_GetWidth(LPDF_Page page);

LPDF_STATUS LPDF_Page_SetHeight(LPDF_Page page, LPDF_UINT16 value);

LPDF_STATUS LPDF_Page_SetWidth(LPDF_Page page, LPDF_UINT16 value);

LPDF_STATUS LPDF_Page_SetLineWidth(LPDF_Page page,LPDF_REAL line_width);


LPDF_STATUS LPDF_Page_SetRGBFill(LPDF_Page page,
                         LPDF_REAL r,
                         LPDF_REAL g,
                         LPDF_REAL b);

LPDF_STATUS LPDF_Page_BeginText(LPDF_Page page);

LPDF_STATUS LPDF_Page_MoveTextPos(LPDF_Page page, LPDF_REAL x, LPDF_REAL y);

LPDF_STATUS LPDF_Page_ShowText(LPDF_Page page, const char *text);

LPDF_STATUS LPDF_Page_EndText(LPDF_Page page);

LPDF_STATUS LPDF_Page_SaveContext(LPDF_Page page);


LPDF_STATUS LPDF_Page_MoveTo (LPDF_Page page,LPDF_UINT16 x,LPDF_UINT16 y);

LPDF_STATUS LPDF_Page_LineTo (LPDF_Page page,LPDF_UINT16 x,LPDF_UINT16 y);

LPDF_STATUS LPDF_Page_RectTo(LPDF_Page page, LPDF_UINT16 x, LPDF_UINT16 y,
    LPDF_UINT16 width, LPDF_UINT16 height);

LPDF_STATUS LPDF_Page_Stroke(LPDF_Page page);

LPDF_STATUS LPDF_Page_SetRGBStroke(LPDF_Page page,
                         LPDF_REAL r,
                         LPDF_REAL g,
                         LPDF_REAL b);

LPDF_Page LPDF_AddPage(LPDF_Doc pdf);

#endif
