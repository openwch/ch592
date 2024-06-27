/********************************** (C) COPYRIGHT *******************************
* File Name          : nvs_flash.h
* Author             : tech7
* Version            : V1.0
* Date               : 2023/03/06
* Description        : 
            
*******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

#ifndef NVS_FLASH_H
#define NVS_FLASH_H

#ifndef SUCCESS
#define SUCCESS	0
#endif

#include "HAL.h"


/*********************************************************************
 * TYPEDEFS
 */

typedef struct __PACKED
{
    uint8_t   check_val_A;
    uint8_t   check_val_B;
    uint8_t   peer_mac[6];
	uint8_t   capacitance;
    uint8_t   check_sum;
}nvs_flash_info_t;

#define CHECK_VAL_A             0x5A
#define CHECK_VAL_B             0xDD

#define NVS_FLASH_INFO_ADDRESS  0x76000-FLASH_ROM_MAX_SIZE
#define NVS_DFU_FLASH_INFO_ADDRESS  0x75000-FLASH_ROM_MAX_SIZE

/*********************************************************************
 * Global Variables
 */
extern nvs_flash_info_t nvs_flash_info;

/*********************************************************************
 * FUNCTIONS
 */
void nvs_flash_init(void);

void nvs_flash_store(void);

/*********************************************************************
*********************************************************************/

#endif
