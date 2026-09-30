/**
 * @brief CLoop SIM-->>PC
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie：首次创建
 *
 */
 
#ifndef CL_H_
#define CL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Smb.h"


int32_t CL_Init(void);

int32_t CL_GetCLoop(uint32_t* pulCurrent);

int32_t CL_Loop(void);

#ifdef __cplusplus
}
#endif

#endif /* CL_H_ */
