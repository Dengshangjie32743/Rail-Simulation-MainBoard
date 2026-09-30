/* begin modified by deng 2023.10.24 */
/**
 * @brief BTM PC-->>SIM/SIM-->>PC
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Zhangmeiyan：首次创建
 *
 */

#ifndef BTM_H_
#define BTM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Smb.h"
#include "sys.h"
#include "use_task.h"

int32_t BTM_Init(void);
int32_t BTM_Loop(uint32_t ulBtmObjIndex);
int32_t BTM_SetBtmData(SMB_BtmMsg_S* pstBtmMsg);
/* begin add by deng 2024年1月30日 */
int32_t BTM_RecvInjectBTMMsg1(SMB_RecvInjectBTMMsg1_S* pstBtmMsg);
/* end add by deng 2024年1月30日 */
int32_t BTM_CleanBtmData(void);

#ifdef __cplusplus
}
#endif

#endif /* BTM_H_ */
