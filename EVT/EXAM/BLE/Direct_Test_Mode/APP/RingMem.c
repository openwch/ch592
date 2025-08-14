/********************************** (C) COPYRIGHT *******************************
* File Name          : RingMem.C
* Author             : WCH
* Version            : V1.0
* Date               : 2025/04/28
* Description        : 
            
*******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <rf_dtm.h>
#include "RingMem.h"
#include "CONFIG.h"
/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      RingMemInit
 *
 * @brief   ��ʼ��
 *
 * @param   none
 *
 * @return  none
 */
void RingMemInit( RingMemParm *Parm, uint8_t *StartAddr, uint32 MaxLen )
{
	Parm->pData		= StartAddr;
	Parm->pWrite 	= StartAddr;
	Parm->pRead  	= StartAddr;
	Parm->RemanentLen = MaxLen;
	Parm->CurrentLen 	= 0;
	Parm->pEnd	  = StartAddr + MaxLen;
  Parm->MaxLen  = MaxLen;
}

/*********************************************************************
 * @fn      RingMemWrite
 *
 * @brief   ����д
 *
 * @param   none
 *
 * @return  none
 */
__HIGH_CODE
uint8_t RingMemWrite( RingMemParm *Parm, uint8_t *pData, uint32 len )
{
	uint32 i,edgelen;

	if( len > Parm->RemanentLen )							//Ҫд���ȳ���
		return !SUCCESS;

	edgelen = Parm->pEnd - Parm->pWrite;			//����ʣ�ൽ�߽�ĳ���

	if( len > edgelen )
	{
		for(i=0; i<edgelen; i++)
		{
			*Parm->pWrite++ = *pData++;
		}
		Parm->pWrite = Parm->pData;
		for(i=0; i<(len-edgelen); i++)
		{
			*Parm->pWrite++ = *pData++;
		}
	}
	else
	{
		for(i=0; i<len; i++)
		{
			*Parm->pWrite++ = *pData++;
		}
	}

	Parm->RemanentLen -= len;
	Parm->CurrentLen 	+= len;
	return SUCCESS;
}

/*********************************************************************
 * @fn      RingMemRead
 *
 * @brief   �����
 *
 * @param   none
 *
 * @return  none
 */
__HIGH_CODE
uint8_t RingMemRead( RingMemParm *Parm, uint8_t *pData, uint32 len )
{
	uint32 i,edgelen;

	if( len > Parm->CurrentLen )						//�ܶ����Ȳ���
		return !SUCCESS;

	edgelen = Parm->pEnd - Parm->pRead;			//����ʣ�ൽ�߽�ĳ���

	if( len > edgelen )
	{
		for(i=0; i<edgelen; i++)
		{
			*pData++ = *Parm->pRead++;
		}
		Parm->pRead = Parm->pData;
		for(i=0; i<(len-edgelen); i++)
		{
			*pData++ = *Parm->pRead++;
		}
	}
	else
	{
		for(i=0; i<len; i++)
		{
			*pData++ = *Parm->pRead++;
		}
	}

	Parm->RemanentLen += len;
	Parm->CurrentLen 	-= len;
	return SUCCESS;
}

/*********************************************************************
 * @fn      RingMemCopy
 *
 * @brief   ���˲�ɾ
 *
 * @param   none
 *
 * @return  none
 */
__HIGH_CODE
uint8_t RingMemCopy( RingMemParm *Parm, uint8_t *pData, uint32 len )
{
	uint32 i,edgelen;
	uint8_t *pRead = Parm->pRead;

	if( len > Parm->CurrentLen )						//�ܸ��Ƶĳ��Ȳ���
		return !SUCCESS;

	edgelen = Parm->pEnd - Parm->pRead;			//����ʣ�ൽ�߽�ĳ���

	if( len > edgelen )
	{
		for(i=0; i<edgelen; i++)
		{
			*pData++ = *pRead++;
		}
		pRead = Parm->pData;
		for(i=0; i<(len-edgelen); i++)
		{
			*pData++ = *pRead++;
		}
	}
	else
	{
		for(i=0; i<len; i++)
		{
			*pData++ = *pRead++;
		}
	}
	return SUCCESS;
}

/*********************************************************************
 * @fn      RingMemDelete
 *
 * @brief   ֱ��ɾ
 *
 * @param   none
 *
 * @return  none
 */
__HIGH_CODE
uint8_t RingMemDelete( RingMemParm *Parm, uint32 len )
{
	uint32 edgelen;

	if( len > Parm->CurrentLen )						//��ɾ�����Ȳ���
		return !SUCCESS;

	edgelen = Parm->pEnd - Parm->pRead;			//����ʣ�ൽ�߽�ĳ���

	if( len > edgelen )
	{
		Parm->pRead+=edgelen;
		Parm->pRead = Parm->pData;
		Parm->pRead+=(len-edgelen);
	}
	else
	{
		Parm->pRead+=len;
	}

	Parm->RemanentLen += len;
	Parm->CurrentLen 	-= len;
	return SUCCESS;
}

/*********************************************************************
 * @fn      RingAddInStart
 *
 * @brief   �򻺳�����ʼ���������
 *
 * @param   none
 *
 * @return  none
 */
__HIGH_CODE
uint8_t RingAddInStart( RingMemParm *Parm, uint8_t *pData, uint32 len )
{
	uint32 i,edgelen;

	if( len > Parm->RemanentLen )							//Ҫд���ȵĳ���
		return !SUCCESS;

	edgelen = Parm->pRead - Parm->pData;			//����ʣ�ൽ�߽�ĳ���

	if( len > edgelen )
	{
		for(i=0; i<edgelen; i++)
		{
			*Parm->pRead-- = *pData++;
		}
		Parm->pRead = Parm->pData + Parm->MaxLen;
		for(i=0; i<(len-edgelen); i++)
		{
			*Parm->pRead-- = *pData++;
		}
	}
	else
	{
		for(i=0; i<len; i++)
		{
			*Parm->pRead-- = *pData++;
		}
	}

	Parm->RemanentLen -= len;
	Parm->CurrentLen 	+= len;
	return SUCCESS;
}

/*********************************************************************
 * @fn      RingCopyOneData
 *
 * @brief   ���׵�ַΪ�������صڼ�������
 *
 * @param   none
 *
 * @return  none
 */
__HIGH_CODE
uint8 RingReturnSingleData( RingMemParm *Parm, uint32 Num )
{
  uint32 edgelen;

  if( Parm->RemanentLen < Parm->MaxLen )
  {
    edgelen = Parm->pEnd - Parm->pRead;
    if( Num >= edgelen )
    {
      return Parm->pData[Num - edgelen];
    }
    else
    {
      return Parm->pRead[Num];
    }
  }
  else
    return 0;
}


__HIGH_CODE
void * __wrap_memset (void *src, int value, size_t size)
{
    char *p = src;
    while (size--) {
        *p++ = value;
    }
    return src;
}

__HIGH_CODE
int __wrap_memcmp (void *src, const void * dst, size_t size)
{
   const char *p1 = src;
   const char *p2 = dst;
   while (size--) {
       if (*p1 != *p2) {
           return *p1 - *p2;
       }
       p1++;
       p2++;
   }
   return 0;
}
/*********************************************************************
*********************************************************************/
