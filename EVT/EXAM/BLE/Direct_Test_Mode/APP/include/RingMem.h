/********************************** (C) COPYRIGHT *******************************
* File Name          : RingMem.h
* Author             : WCH
* Version            : V1.0
* Date               : 2025/04/28
* Description        : 
            
*******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
#include "CONFIG.h"

#ifndef RINGMEM_H
#define RINGMEM_H

#ifndef SUCCESS
#define SUCCESS	0
#endif

/*********************************************************************
 * TYPEDEFS
 */
 
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;

//缓冲区结构体
typedef struct 
{
         uint8   *pData;		    //缓冲区首地址
         uint8 	 *pWrite;           //写指针
         uint8 	 *pRead;		    //读指针
         uint8   *pEnd;			    //缓冲区末地址
volatile uint32  RemanentLen;       //剩余空间大小
volatile uint32  CurrentLen;        //已用空间大小
volatile uint32  MaxLen;      	    //总空间大小
}RingMemParm;

/*********************************************************************
 * Global Variables
 */

/*********************************************************************
 * FUNCTIONS
 */

extern void RingMemInit( RingMemParm *Parm, uint8 *StartAddr, uint32 MaxLen );

extern uint8 RingMemWrite( RingMemParm *Parm, uint8 *pData, uint32 len );

extern uint8 RingMemRead( RingMemParm *Parm, uint8 *pData, uint32 len );

extern uint8 RingMemCopy( RingMemParm *Parm, uint8 *pData, uint32 len );

extern uint8 RingMemDelete( RingMemParm *Parm, uint32 len );

extern uint8 RingAddInStart( RingMemParm *Parm, uint8 *pData, uint32 len );

extern uint8 RingReturnSingleData( RingMemParm *Parm, uint32 Num );

extern int __wrap_memcmp (void *src, const void * dst, size_t size);

extern void * __wrap_memset (void *src, int value, size_t size);
/*********************************************************************
*********************************************************************/

#endif
