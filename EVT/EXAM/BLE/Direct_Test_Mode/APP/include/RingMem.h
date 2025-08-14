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

//�������ṹ��
typedef struct 
{
         uint8   *pData;		    //�������׵�ַ
         uint8 	 *pWrite;           //дָ��
         uint8 	 *pRead;		    //��ָ��
         uint8   *pEnd;			    //������ĩ��ַ
volatile uint32  RemanentLen;       //ʣ��ռ��С
volatile uint32  CurrentLen;        //���ÿռ��С
volatile uint32  MaxLen;      	    //�ܿռ��С
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
