/**
 * @brief RADAR：PC-->>SIM
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie
 *
 */

#ifndef RADAR_H_
#define RADAR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Smb.h"

/**
*   @brief 雷达数据帧格式
*/
typedef struct
{
    uint8_t head[2U];           /**< 帧头             */
    uint8_t seq;                /**< 序列号           */
    uint8_t v[2U];              /**< 速度值，16Bit    */
    uint8_t s1[2U];             /**< 距离1，单位10厘米*/
    uint8_t s2[2U];             /**< 距离2，单位100米 */
    uint8_t PWR40;              /**< 40°接收信号值    */
    uint8_t PWR50;              /**< 50°接收信号值    */
    uint8_t status;             /**< 状态值           */
    uint8_t fcs;                /**< 帧校验           */
}AppRadarFrame_ST_M;

int32_t RADAR_Init(void);
int32_t RADAR_SetVelocity(int32_t lVelocity, uint32_t ulPulseDistance);
uint32_t RADAR_GetPulseDistance(void);
int32_t RADAR_GetVelocity(void);
int32_t RADAR_SetInject1(SMB_RecvInjectRadarMsg1_S* pstMsg);
int32_t RADAR_SetInject2(SMB_RecvInjectRadarMsg2_S* pstMsg);
void RADAR_Loop(void);

#ifdef __cplusplus
}
#endif

#endif /* RADAR_H_ */


