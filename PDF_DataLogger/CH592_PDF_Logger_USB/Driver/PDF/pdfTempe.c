/*
*生成pdf模板代码:
*Created on: 2023-07-26
*Author: WCH
*/
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pdfFile.h"
#include "pdf.h"
#include "flash_info.h"

//#define CONFIG_PDFTEMPE_DEBUG
#ifdef CONFIG_PDFTEMPE_DEBUG
#define LOG_INFO(...)           PRINT(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

// 首条记录时间
uint8_t timeStart[6] = { 23,  7,  27,  12,  12,  12};
#define   YEAR_OFFSET          0
#define   MONTH_OFFSET         1
#define   DAY_OFFSET           2
#define   HOUR_OFFSET          3
#define   MINUTE_OFFSET        4
#define   SECOND_OFFSET        5

const char centigrade[3] ={ 0xA1, 0xE6,'\0' };  // ℃, temperature unit

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

#define IFLASH_TEMP_START_ADDR   (268*1024)
LPDF_REAL *pdf_get_temperature_data( LPDF_REAL *pBuf,LPDF_UINT offset, LPDF_UINT *num  )
{
    int16_t temp;

//    EEPROM_READ( IFLASH_TEMP_START_ADDR + offset*sizeof(LPDF_REAL), pBuf, *num * sizeof(LPDF_REAL) );
    FLASH_ROM_READ(IFLASH_TEMP_START_ADDR + offset*sizeof(LPDF_REAL), pBuf, *num * sizeof(LPDF_REAL) );
    for(uint16_t i=0; i<*num; ++i)
    {
        temp = ((int16_t *)pBuf)[2*i];
        pBuf[i] = ((float)temp)/10.0;
        LOG_INFO("T:%f\n", pBuf[i]);
    }

    return pBuf;
}

#define IFLASH_HUMI_START_ADDR   (268*1024)
LPDF_REAL *pdf_get_humidity_data( LPDF_REAL *pBuf,LPDF_UINT offset, LPDF_UINT *num  )
{
    int16_t temp;

//    EEPROM_READ( IFLASH_HUMI_START_ADDR + offset*sizeof(LPDF_REAL), pBuf, *num * sizeof(LPDF_REAL) );
    FLASH_ROM_READ(IFLASH_HUMI_START_ADDR + offset*sizeof(LPDF_REAL), pBuf, *num * sizeof(LPDF_REAL) );
    for(uint16_t i=0; i<*num; ++i)
    {
        temp = ((int16_t *)pBuf)[2*i+1];
        pBuf[i] = ((float)temp)/10.0;
        LOG_INFO("H:%f\n", pBuf[i]);
    }

    return pBuf;
}


void pdf_temperature_init( LPDF_Info info )
{
    memset( info, 0, sizeof(LPDF_Info));
    strcpy(info->FileName,"template.pdf"); //header
    strcpy(info->FileVersion, "v1.4");
    strcpy(info->WebUrl, "www.wch.cn");

    info->HumiTotalDataNum = info->TempTotalDataNum = DeviceInfo.MeasureNum; //total number of data
    info->tempGetDataCb = pdf_get_temperature_data; //data read callback function
    info->humiGetDataCb = pdf_get_humidity_data; //data read callback function
    info->pDataBuf = DataBuf;  //Used to cache temperature and humidity data read from Flash (pdf_get_temperature_data/pdf_get_humidity_data)
    info->DataBufLen = DATA_MAX_LEN;

    info->isGraphEnable = LPDF_TRUE; //Show graph in PDF
    info->isTableEnable = LPDF_TRUE; //Show all data in PDF

    // Device Information
    info->DeviceInfo.pInfo = device_info; //Displayed in the first column (Device Information), Items set to NULL cannot be modified
    strcpy(info->DeviceInfo.ProductType,     "ZZ-10A");
    strcpy(info->DeviceInfo.SerialNumber,    "ABCD1234");
    strcpy(info->DeviceInfo.ProtectionGrade, "IP67");
    strcpy(info->DeviceInfo.ProbeType,       "temperature&humidity" );
    info->DeviceInfo.StorageSpace = 10000; //Maximum amount of data stored
    info->DeviceInfo.MinMeauRange = -40; //Measuring range
    info->DeviceInfo.MaxMeauRange = 70;

    // Trip Information
    info->TripInfo.pInfo = trip_info; //Displayed in the second column (Trip Information), Items set to NULL cannot be modified
    strcpy(info->TripInfo.Order,       "A-0-B-1-001");
    strcpy(info->TripInfo.Shipper,     "anonymity");
    strcpy(info->TripInfo.City,        "NanJing");
    strcpy(info->TripInfo.Carrier,     "lobster");
    strcpy(info->TripInfo.Receiver,    "abc");
    strcpy(info->TripInfo.Signature,   "xxx");

    // Logger Configuration
    info->ConfigInfo.pInfo = logger_conf; //Displayed in the third column (Logger Configuration), Items set to NULL cannot be modified
    strcpy(info->ConfigInfo.StartMode, "Key Start");
    strcpy(info->ConfigInfo.StopMode,  "Key Stop + Full Stop");
    info->ConfigInfo.IsKeyStop = LPDF_TRUE;
    info->ConfigInfo.IsTone = LPDF_FALSE;
    info->ConfigInfo.MaxTempAlarm = DeviceInfo.PdfParam.MaxTempAlarm; //temperature safe range
    info->ConfigInfo.MinTempAlarm = DeviceInfo.PdfParam.MinTempAlarm;

    info->ConfigInfo.StartDelayed = 60;
    info->ConfigInfo.Interval = DeviceInfo.PdfParam.MeasureInterval*60;
    strcpy(info->ConfigInfo.TempUint, centigrade );
    info->ConfigInfo.IsAlarmTone = LPDF_FALSE;
    info->ConfigInfo.MaxHumiAlarm = DeviceInfo.PdfParam.MaxHumiAlarm; //humidity safe range
    info->ConfigInfo.MinHumiAlarm = DeviceInfo.PdfParam.MinHumiAlarm;

    // Statistical Information
    info->StatisInfo.RecordNum = info->TempTotalDataNum + info->HumiTotalDataNum;  //Displayed in the fourth column (Statistical Information)
    info->StatisInfo.FirstRecordYear   = (LPDF_UINT8)DeviceInfo.StartTime.Year - 2000;  //timeStart[YEAR_OFFSET]; //Data recording start time
    info->StatisInfo.FirstRecordMonth  = DeviceInfo.StartTime.Month;                    //timeStart[MONTH_OFFSET];
    info->StatisInfo.FirstRecordDay    = DeviceInfo.StartTime.Day;                      //timeStart[DAY_OFFSET];
    info->StatisInfo.FirstRecordHour   = DeviceInfo.StartTime.Hour;                     //timeStart[HOUR_OFFSET];
    info->StatisInfo.FirstRecordMinute = DeviceInfo.StartTime.Minute;                   //timeStart[MINUTE_OFFSET];
    info->StatisInfo.FirstRecordSec    = DeviceInfo.StartTime.Second;                   //timeStart[SECOND_OFFSET];
}

int pdf_create( char *file_name )
{
    LPDF_Init param;

    LOG_INFO("file_name:%s\n",file_name);
    open_file( file_name ); //Open file from internal Flash

    param.pPdfBuf = PdfBuf; //Used to cache generated file data (pdf_data_proces)
    param.pdfProcessCb = pdf_data_proces; //Callback function to save file data to disk
    LPDF_InitParam( &param );

#if 1 // Create a cold chain PDF template defined by Qinheng
    {
        static LPDF_Template_Info temperInfo;

        pdf_temperature_init( &temperInfo ); //Initialize the content displayed in the PDF

        memset( &pdf,0,sizeof(LPDF_Doc_Rec));
        temperInfo.pdf = LPDF_New( &pdf );
        pdf_template_creat( &temperInfo );
        LPDF_SaveToFile( temperInfo.pdf );
    }
#else //Customize a PDF template
    {
        LPDF_Doc pTemp;
        LPDF_Page page[1] = { 0 }; //One page PDF array

        //draw a line
        memset( &pdf,0,sizeof(LPDF_Doc_Rec));
        pTemp = LPDF_New( &pdf );
        page[0] = LPDF_AddPage( pTemp );
        LPDF_Page_SetLineWidth(page[0],1.0);
        LPDF_Page_MoveTo(page[0], 25,  810);
        LPDF_Page_LineTo(page[0], 575, 810);
        LPDF_Page_MoveTo(page[0], 25,  20);
        LPDF_Page_LineTo(page[0], 575, 20);
        LPDF_Page_Stroke(page[0]); //The line drawing operation ends. Clear the relevant parameters of the configuration.

        //header
        LPDF_Page_SetRGBFill(page[0], 0.0, 0.0, 0.0);
        LPDF_Page_BeginText(page[0]);
        LPDF_Page_MoveTextPos(page[0], 30, 815);
        LPDF_Page_SetFontAndSize(page[0], "SimSun", 7);
        LPDF_Page_ShowText(page[0], file_name );
        LPDF_Page_MoveTextPos(page[0], 480, 0);
        LPDF_Page_ShowText(page[0], "v1.4");
        LPDF_Page_EndText(page[0]); //The text operation ends. Clear the relevant parameters of the configuration.

        //draw thick lines
        LPDF_Page_SetRGBStroke(page[0], 0.8, 0.8, 0.8);
        LPDF_Page_SetLineWidth(page[0], 24.0);
        LPDF_Page_MoveTo(page[0], 30,  780);
        LPDF_Page_LineTo(page[0], 565, 780);
        LPDF_Page_Stroke(page[0]); //The line drawing operation ends. Clear the relevant parameters of the configuration.

        //display text
        LPDF_Page_SetRGBFill(page[0], 0, 0.0, 0);
        LPDF_Page_BeginText(page[0]);
        LPDF_Page_MoveTextPos(page[0], (565/2), 780-3);
        LPDF_Page_SetFontAndSize(page[0], "SimSun", 18);
        LPDF_Page_ShowText(page[0], "demo");
        LPDF_Page_MoveTextPos(page[0], -200, -200 );
        LPDF_Page_SetRGBFill(page[0], 1.0, 0.0, 0.0);
        LPDF_Page_SetFontAndSize(page[0], "SimSun", 28);
        LPDF_Page_ShowText(page[0], "hello world");
        LPDF_Page_EndText(page[0]); //The text operation ends. Clear the relevant parameters of the configuration.

        //draw a rectangle
        LPDF_Page_SetRGBStroke(page[0], 0, 0, 0);
        LPDF_Page_SetLineWidth(page[0], 1.0);
        LPDF_Page_MoveTo(page[0], 50, 45);
        LPDF_Page_RectTo(page[0], 50, 45, 512, 400);
        LPDF_Page_Stroke(page[0]); //The line drawing operation ends. Clear the relevant parameters of the configuration.

        //draw grid
        LPDF_Page_SetLineWidth(page[0], 0.8);
        for( int i=0;i<12;i++ ){
            if( (i%3) == 0)LPDF_Page_SetRGBStroke(page[0], 0.5, 0.0, 0.0);
            if( (i%3) == 1)LPDF_Page_SetRGBStroke(page[0], 0.0, 0.5, 0.0);
            if( (i%3) == 2)LPDF_Page_SetRGBStroke(page[0], 0.0, 0.0, 0.5);
            LPDF_Page_SetLineWidth(page[0], 30.0);
            LPDF_Page_MoveTo(page[0], 50, 80+30*i);
            LPDF_Page_LineTo(page[0], 50+512, 80+30*i );
            LPDF_Page_Stroke(page[0]);  //The line drawing operation ends. Clear the relevant parameters of the configuration.
        }

        //footer
        LPDF_Page_SetRGBFill(page[0], 0.0, 0.0, 0.0);
        LPDF_Page_BeginText(page[0]);
        LPDF_Page_MoveTextPos(page[0], 40, 10);
        LPDF_Page_SetFontAndSize(page[0], "SimSun", 7);
        LPDF_Page_ShowText(page[0], "www.wch.cn");
        LPDF_Page_MoveTextPos(page[0], 490, 0);
        LPDF_Page_ShowText(page[0], "1/1");
        LPDF_Page_EndText(page[0]); //The text operation ends. Clear the relevant parameters of the configuration.

        //save
        LPDF_Page_SaveContext(page[0]); //The PDF operation for this page is finished. Clear the relevant parameters of the configuration.
        LPDF_SaveToFile( pTemp); //The entire PDF file operation ends. Clear the relevant parameters of the configuration.
  }
#endif

    close_file( );
    return 0;
}

