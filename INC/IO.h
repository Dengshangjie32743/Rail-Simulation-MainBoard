/**
 * @brief IO
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie：首次创建
 *
 */

#ifndef IO_H_
#define IO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Smb.h"
#include "QueueInc.h"
#include "SLib.h"

/* begin modified by deng 2023.11.6 */
#define   IO_OUTPUT_BEGIN_CANID   0X130 /**<仿真输出输出板开始ID */
#define   IO_OUTPUT_END_CANID     0X13B /**<仿真输出输出板结束ID */
#define   IO_OUTPUT_PACK_CANID    0XAA  /**<仿真输出输出板总包ID */

#define   IO_INPUT_BEGIN_CANID    0X210 /**<仿真输入板开始ID */
#define   IO_INPUT_END_CANID      0X21B /**<仿真输入板结束ID */
 /**<仿真输入板结束ID */
/* end modified by deng 2023.11.6  */

/* begin add by deng 2023年11月30日 */
#define IO_INFO_NUM    64u
#define IO_DATA_LEN    8u
/* end add by deng 2023年11月30日 */

int32_t IO_Init(void);

int32_t IO_SendIoInfo(SMB_IoBoardInfo_S astIoBoardInfo[], uint32_t ulBoardNum);

int32_t IO_GetIoInfo(SMB_IoBoardInfo_S astIoBoardInfo[], uint32_t *pulBoardNum);

#ifdef __cplusplus
}
#endif

#endif /* IO_H_ */
