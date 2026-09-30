/**
 * @brief BTM PC-->>SIM/SIM-->>PC
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie
 *
 */

#ifndef CRC_H_
#define CRC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "TypeInc.h"


uint32_t S_Crc32I(const uint8_t* pdata, uint32_t datalen);
uint32_t S_Crc32C(const uint8_t* pdata, uint32_t datalen);

uint16_t S_CharToUint16(const uint8_t aucData[], bool_t bIsBigEndian);
uint32_t S_CharToUint32(const uint8_t aucData[], bool_t bIsBigEndian);
void S_Uint16ToChar(uint16_t usData, bool_t bIsBigEndian, uint8_t aucData[]);
void S_Uint32ToChar(uint32_t ulData, bool_t bIsBigEndian, uint8_t aucData[]);
/* begin modified by deng 2023.10.27 */
void *S_Memset(void *str, uint8_t c, uint32_t n);
void *S_MemZero(void *str, uint32_t n);
uint32_t S_GetLocalTime(void);
/* end modified by deng 2023.10.27 */
/* begin modified by deng 2023.11.6 */
uint8_t S_CheckXOR(uint8_t* pucDataBuf, int32_t uDataLen);
uint32_t S_ABS(int32_t lData) ;
/* end modified by deng 2023.11.6  */
#ifdef __cplusplus
}
#endif

#endif /* CRC_H_ */
