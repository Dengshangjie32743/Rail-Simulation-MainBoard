/**
 * @brief CPUusage
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie：首次创建
 *
 */

#ifndef CPU_USAGE_H_
#define CPU_USAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Smb.h"
#include "SLib.h"

/* begin modified by deng 2023.11.6 */

/* end add by deng 2023年11月30日 */
void CpuUsageHook(void);
void CpuUsage_LOOP(void);

#ifdef __cplusplus
}
#endif

#endif /* CPU_USAGE_H_ */
