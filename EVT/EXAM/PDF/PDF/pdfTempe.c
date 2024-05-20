/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/07/26
 * Description        : Generate PDF template code
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* Header Files */
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pdfFile.h"
#include "pdf.h"


// First record time
uint8_t timeStart[6] = { 23,  7,  27,  12,  12,  12};
#define   YEAR_OFFSET          0
#define   MONTH_OFFSET         1
#define   DAY_OFFSET           2
#define   HOUR_OFFSET          3
#define   MINUTE_OFFSET        4
#define   SECOND_OFFSET        5

const char centigrade[3] ={ 0xA1, 0xE6,'\0' };  // ℃

const char *device_info[] = {
    "Device Information",
    "Model#:",
    "Serial#:",
    "IP Grade:",
    "Probe Type:",
    "Storage:",
    NULL,    // MeauRange:
};

const char *trip_info[] = {
    "Trip Information",
    "Order:",
    "Shipper:",
    "City:",
    "Carrier:",
    "Receiver:",
    "Signature:",
};

const char *logger_conf[] = {
    "Logger Configuration",
    "Start Mode:",
    "Stop Mode:",
    "Key Stop:",
    "Warning Tone:",
    NULL,    // Temp Limit:
    "Start Delay:",
    NULL,    // Interval:
    NULL,    // Temp Unit:
    "Alarm Tone:",
    "NULL",  // Humi Limit:
};
#define PDF_BUF_LEN     256
#define DATA_MAX_LEN    256


static LPDF_Doc_Rec pdf;
static LPDF_INT8 PdfBuf[PDF_BUF_LEN] = { 0 };
static LPDF_REAL DataBuf[DATA_MAX_LEN];

/*********************************************************************
 * @fn      pdf_get_temperature_data
 *
 * @brief   The purpose of this function is to obtain a segment of temperature data from the specified memory address offset, process it, and store it in the given buffer.
 *
 *          pBuf: Pointer to the buffer where temperature data is stored;
 *          offset: The starting offset address of temperature data in memory;
 *          num: Pointer to an integer pointer used to store the number of temperature data obtained.
 *
 * @param   pBuf - 数据缓冲区
 *          offset - 偏移量
 *          num - 数据量
 *
 * @return  a pointer to the buffer where temperature data is stored, i.e. pBuf
 */
LPDF_REAL *pdf_get_temperature_data( LPDF_REAL *pBuf,LPDF_UINT offset, LPDF_UINT *num  )
{
  for (int i = 0;i < *num;i++) {
    pBuf[i] = *((uint32_t*)((offset+i)*sizeof(LPDF_REAL)));
    pBuf[i] = (((uint64_t)pBuf[i]*(45-1))>>32)+1;
  }
  return pBuf;
}

/*********************************************************************
 * @fn      pdf_get_humidity_data
 *
 * @brief   The purpose of this function is to obtain a segment of humidity data from the specified memory address offset, process it, and store it in the given buffer.
 *
 *          pBuf: Pointer to the buffer where temperature data is stored;
 *          offset: The starting offset address of temperature data in memory;
 *          num: Pointer to an integer pointer used to store the number of temperature data obtained.
 *
 * @param   pBuf - 数据缓冲区
 *          offset - 偏移量
 *          num - 数据量
 *
 * @return  a pointer to the buffer where temperature data is stored, i.e. pBuf
 */
LPDF_REAL *pdf_get_humidity_data( LPDF_REAL *pBuf,LPDF_UINT offset, LPDF_UINT *num  )
{
  for (int i = 0;i < *num;i++) {
    pBuf[i] = *((uint32_t*)((offset+i)*sizeof(LPDF_REAL)));
    pBuf[i] = (((uint64_t)pBuf[i]*(70-30))>>32)+30;
  }
  return pBuf;
}

/*********************************************************************
 * @fn      pdf_temperature_init
 *
 * @brief   Complete the initialization of various information in the PDF document
 *
 * @param   info - 指向LPDF_info结构体的指针，存放PDF模板的各项属性
 *
 * @return  none
 */
void pdf_temperature_init( LPDF_Info info )
{
  memset( info, 0, sizeof(LPDF_Info));
  strcpy(info->FileName,"template.pdf");
  strcpy(info->FileVersion, "v1.4");
  strcpy(info->WebUrl, "www.wch.cn");

  info->HumiTotalDataNum = info->TempTotalDataNum = 500;
  info->tempGetDataCb = pdf_get_temperature_data;
  info->humiGetDataCb = pdf_get_humidity_data;
  info->pDataBuf = DataBuf;
  info->DataBufLen = DATA_MAX_LEN;

  info->isGraphEnable = LPDF_TRUE;
  info->isTableEnable = LPDF_TRUE;

  // Device Information
  info->DeviceInfo.pInfo = device_info;
  strcpy(info->DeviceInfo.ProductType,     "ZZ-10A");
  strcpy(info->DeviceInfo.SerialNumber,    "ABCD1234");
  strcpy(info->DeviceInfo.ProtectionGrade, "IP67");
  strcpy(info->DeviceInfo.ProbeType,       "temperature&humidity" );
  info->DeviceInfo.StorageSpace = 10000;
  info->DeviceInfo.MinMeauRange = -40;
  info->DeviceInfo.MaxMeauRange = 70;

  // Trip Information
  info->TripInfo.pInfo = trip_info;
  strcpy(info->TripInfo.Order,       "A-0-B-1-001");
  strcpy(info->TripInfo.Shipper,     "anonymity");
  strcpy(info->TripInfo.City,        "NanJing");
  strcpy(info->TripInfo.Carrier,     "lobster");
  strcpy(info->TripInfo.Receiver,    "abc");
  strcpy(info->TripInfo.Signature,   "xxx");

  // Logger Configuration
  info->ConfigInfo.pInfo = logger_conf;
  strcpy(info->ConfigInfo.StartMode, "Key Start");
  strcpy(info->ConfigInfo.StopMode,  "Key Stop + Full Stop");
  info->ConfigInfo.IsKeyStop = LPDF_TRUE;
  info->ConfigInfo.IsTone = LPDF_FALSE;
  info->ConfigInfo.MaxTempAlarm = 20;
  info->ConfigInfo.MinTempAlarm = -10;

  info->ConfigInfo.StartDelayed = 60;
  info->ConfigInfo.Interval = 100;
  strcpy(info->ConfigInfo.TempUint, centigrade );
  info->ConfigInfo.IsAlarmTone = LPDF_FALSE;
  info->ConfigInfo.MaxHumiAlarm = 70;
  info->ConfigInfo.MinHumiAlarm = 30;

   // Statistical Information
  info->StatisInfo.RecordNum = info->TempTotalDataNum + info->HumiTotalDataNum;
  info->StatisInfo.FirstRecordYear   = timeStart[YEAR_OFFSET];
  info->StatisInfo.FirstRecordMonth  = timeStart[MONTH_OFFSET];
  info->StatisInfo.FirstRecordDay    = timeStart[DAY_OFFSET];
  info->StatisInfo.FirstRecordHour   = timeStart[HOUR_OFFSET];
  info->StatisInfo.FirstRecordMinute = timeStart[MINUTE_OFFSET];
  info->StatisInfo.FirstRecordSec    = timeStart[SECOND_OFFSET];
}

/*********************************************************************
 * @fn      pdf_create
 *
 * @brief   Create a PDF document
 *
 * @param   file_name - 文件名
 *
 * @return  none
 */
int pdf_create( char *file_name )
{
    LPDF_Init param;

    printf("file_name:%s\n",file_name);
    open_file( file_name );                  // Call open_ The file function opens the specified file

    param.pPdfBuf = PdfBuf;
    param.pdfProcessCb = pdf_data_proces;
    LPDF_InitParam( &param );                // Calling LPDF_ InitParam function, initializing param

#if 1
  {
      static LPDF_Template_Info temperInfo;
      pdf_temperature_init( &temperInfo );

      memset( &pdf,0,sizeof(LPDF_Doc_Rec));  // Use the memset function to fill all the memory space of the PDF variable with 0
      temperInfo.pdf = LPDF_New( &pdf );     // Assign temperInfo.pdf to the PDF variable to create a new PDF document.
      pdf_template_creat( &temperInfo );     // Call pdf_template_creat function generates a PDF template based on the information in temperInfo
      LPDF_SaveToFile( temperInfo.pdf );     // Calling LPDF_SaveToFile function to save temperInfo.pdf to a file
  }
#else
  {
      LPDF_Doc pTemp;
      LPDF_Page page[1] = { 0 };

      memset( &pdf,0,sizeof(LPDF_Doc_Rec));
      pTemp = LPDF_New( &pdf );
      page[0] = LPDF_AddPage( pTemp );
      LPDF_Page_SetLineWidth(page[0],1.0);             // Set the width of page lines to 1.0
      LPDF_Page_MoveTo(page[0], 25,  810);
      LPDF_Page_LineTo(page[0], 575, 810);             // Draw the top and bottom horizontal lines of the page using the LPDF_Page_MoveTo and LPDF_Page_LineTo function
      LPDF_Page_MoveTo(page[0], 25,  20);
      LPDF_Page_LineTo(page[0], 575, 20);
      LPDF_Page_Stroke(page[0]);                       // Display the drawn lines on the page

      // 页眉
      LPDF_Page_SetRGBFill(page[0], 0.0, 0.0, 0.0);    // Set the font color of the page to black
      LPDF_Page_BeginText(page[0]);                    // Start drawing text
      LPDF_Page_MoveTextPos(page[0], 30, 815);         // Set the starting position of the text
      LPDF_Page_SetFontAndSize(page[0], "SimSun", 7);  // Set the font and size of the text
      LPDF_Page_ShowText(page[0], file_name );         // Display file name and version number
      LPDF_Page_MoveTextPos(page[0], 480, 0);
      LPDF_Page_ShowText(page[0], "v1.4");
      LPDF_Page_EndText(page[0]);                      // End Text Draw

      LPDF_Page_SetRGBStroke(page[0], 0.8, 0.8, 0.8);  // Set the line color of the page to gray
      LPDF_Page_SetLineWidth(page[0], 24.0);           // Set the line width of the page to 24.0
      LPDF_Page_MoveTo(page[0], 30,  780);
      LPDF_Page_LineTo(page[0], 565, 780);
      LPDF_Page_Stroke(page[0]);

      LPDF_Page_SetRGBFill(page[0], 0, 0.0, 0);
      LPDF_Page_BeginText(page[0]);
      LPDF_Page_MoveTextPos(page[0], (565/2), 780-3);
      LPDF_Page_SetFontAndSize(page[0], "SimSun", 18);
      LPDF_Page_ShowText(page[0], "demo");
      LPDF_Page_MoveTextPos(page[0], -200, -200 );
      LPDF_Page_SetRGBFill(page[0], 1.0, 0.0, 0.0);
      LPDF_Page_SetFontAndSize(page[0], "SimSun", 28);
      LPDF_Page_ShowText(page[0], "hello world");
      LPDF_Page_EndText(page[0]);

      LPDF_Page_SetRGBStroke(page[0], 0, 0, 0);
      LPDF_Page_SetLineWidth(page[0], 1.0);
      LPDF_Page_MoveTo(page[0], 50, 45);
      LPDF_Page_RectTo(page[0], 50, 45, 512, 400);
      LPDF_Page_Stroke(page[0]);

      LPDF_Page_SetLineWidth(page[0], 0.8);
      for( int i=0;i<12;i++ ){
          if( (i%3) == 0)LPDF_Page_SetRGBStroke(page[0], 0.5, 0.0, 0.0);
          if( (i%3) == 1)LPDF_Page_SetRGBStroke(page[0], 0.0, 0.5, 0.0);
          if( (i%3) == 2)LPDF_Page_SetRGBStroke(page[0], 0.0, 0.0, 0.5);
          LPDF_Page_SetLineWidth(page[0], 30.0);
          LPDF_Page_MoveTo(page[0], 50, 80+30*i);
          LPDF_Page_LineTo(page[0], 50+512, 80+30*i );
          LPDF_Page_Stroke(page[0]);
      }

      // 页脚
      LPDF_Page_SetRGBFill(page[0], 0.0, 0.0, 0.0);
      LPDF_Page_BeginText(page[0]);
      LPDF_Page_MoveTextPos(page[0], 40, 10);
      LPDF_Page_SetFontAndSize(page[0], "SimSun", 7);
      LPDF_Page_ShowText(page[0], "www.wch.cn");
      LPDF_Page_MoveTextPos(page[0], 490, 0);
      LPDF_Page_ShowText(page[0], "1/1");
      LPDF_Page_EndText(page[0]);

      LPDF_Page_SaveContext(page[0]);
      LPDF_SaveToFile( pTemp );
  }
#endif
  close_file( );
  return 0;
}

