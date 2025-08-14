#ifndef APP_TMOS_H_
#define APP_TMOS_H_

#include "CH59x_common.h"

#define DEALDATA_EVT         0x0001
#define WAKEUP_DATA_DEAL_EVT 0x0002
#define DEBUG_PRINT_EVENT    0x0004
#define TKY_KEEPALIVE_EVENT  0x0008

/************************TOUCH_KEY_DEFINE****************************/
#define TOUCH_KEY_ELEMENTS                  (TKY_MAX_QUEUE_NUM)
#define TOUCH_KEY_CHS                       0,1,2,3,4,5,6,7

void PeriodicDealData(void);
void touch_on_TMOS_init(void);
void tky_on_TMOS_dataProcess(void);
void tky_DealData_stop(void);
void tky_DealData_start(void);

#endif
