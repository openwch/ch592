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
#ifndef __PDF_H
#define __PDF_H


#include "pdflib.h"

typedef LPDF_REAL *(*pFnPdfGetData)( LPDF_REAL *pBuf,LPDF_UINT offset, LPDF_UINT *num  );

typedef struct
{
    const char **pInfo;
    char ProductType[16];
    char SerialNumber[16];
    char ProtectionGrade[8];
    char ProbeType[24];
    LPDF_UINT StorageSpace;
    LPDF_REAL MaxMeauRange;    //≤‚¡ø∑∂Œß
    LPDF_REAL MinMeauRange;
    LPDF_UINT RecodeInterval;
}LPDF_Device_Info;

typedef struct
{
    const char **pInfo;
    char Order[12];
    char Shipper[12];
    char City[12];
    char Carrier[12];
    char Receiver[12];
    char Signature[12];
}LPDF_Trip_Info;

typedef struct
{
    const char **pInfo;
    char StartMode[32];
    char StopMode[32];
    LPDF_BOOL IsKeyStop;
    LPDF_BOOL IsTone;
    LPDF_REAL MaxTempAlarm;
    LPDF_REAL MinTempAlarm;
    LPDF_UINT StartDelayed;
    LPDF_UINT Interval;
    char TempUint[4];
    LPDF_BOOL IsAlarmTone;
    LPDF_REAL MaxHumiAlarm;
    LPDF_REAL MinHumiAlarm;
}LPDF_Config_Info;

typedef struct
{
    const char **pInfo;
    LPDF_UINT RecordNum;
    LPDF_UINT8 FirstRecordYear;
    LPDF_UINT8 FirstRecordMonth;
    LPDF_UINT8 FirstRecordDay;
    LPDF_UINT8 FirstRecordHour;
    LPDF_UINT8 FirstRecordMinute;
    LPDF_UINT8 FirstRecordSec;
    LPDF_UINT8 LastRecordYear;
    LPDF_UINT8 LastRecordMonth;
    LPDF_UINT8 LastRecordDay;
    LPDF_UINT8 LastRecordHour;
    LPDF_UINT8 LastRecordMinute;
    LPDF_UINT8 LastRecordSec;
    LPDF_REAL MaxTemperature;
    LPDF_REAL MinTemperature;
    LPDF_REAL AverTemperature;
    LPDF_BOOL IsTemperatureAlarm;
    char RecordDuration[32];
    char LastRecordTime[32];
    LPDF_REAL MaxHumidity;
    LPDF_REAL MinHumidity;
    LPDF_REAL AverHumidity;
    LPDF_BOOL IsHumidityAlarm;
}LPDF_Statistical_Info;

typedef struct
{
    LPDF_Doc pdf;
    char FileName[32]; //header
    char FileVersion[8];
    char WebUrl[32];
    LPDF_UINT TempTotalDataNum; //total number of data
    LPDF_UINT HumiTotalDataNum; //total number of data

    pFnPdfGetData tempGetDataCb;
    pFnPdfGetData humiGetDataCb;
    LPDF_REAL *pDataBuf;
    LPDF_UINT DataBufLen;
    LPDF_BOOL isGraphEnable;
    LPDF_BOOL isTableEnable;
    LPDF_Device_Info DeviceInfo;
    LPDF_Trip_Info TripInfo;
    LPDF_Config_Info ConfigInfo;
    LPDF_Statistical_Info StatisInfo;
}LPDF_Template_Info;

typedef LPDF_Template_Info* LPDF_Info;

LPDF_STATUS pdf_template_creat( LPDF_Template_Info *pTemp );

#endif
