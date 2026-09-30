/**
 * @brief PC-->>SIM
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie：首次创建
 *
 */

#ifndef SPD_H_
#define SPD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Smb.h"

#define CHANNEL_COUNT 3U
/* begin modified by deng 2023.11.6 */
#define SPEED_FREQ_MAX 20000U
#define SPEED_FREQ_MIN 0U

/* begin modified by deng 2025年3月1日 */
//#define SPEED_FREQ_TOTAL 50000 //速度计频率总值为1.25MHz
#define SPEED_FREQ_TOTAL 1250000 //速度计频率总值为1.25MHz
/* end modified by deng 2025年3月1日 */

/**
 * @brief 速度计数据结构体    //todo：该结构体不满足四字节对齐，待讨论 denghsangjie
 */
#pragma pack (1)
typedef struct {
    uint8_t ucSPDHeadA;               /**< 报文头1: 0xFF */
    uint8_t ucSPDHeadB;               /**< 报文头2: 0xFE */
    uint8_t ucChannelValid;           /**< 通道有效性bit0~bit5对应6个通道：1为输出延时信号，0为输出占空比调整信号，其中bit0电机1通道1，bit1电机1通道2，，，bit5表示电机2通道3；bit6~bit7暂定为0） */
    uint32_t ausMotorAChannelFreq[3]; /**< 电机A通道频率 */
    uint32_t ausMotorBChannelFreq[3]; /**< 电机B通道频率 */
    uint16_t ausMotorAPhase[3];       /**< 电机A通道相位 */
    uint16_t ausMotorBPhase[3];       /**< 电机B通道相位 */
    uint8_t aucMotorADutyCycle[3];    /**< 电机A通道占空比，默认为50% ,(0~255对应0~100%)*/
    uint8_t aucMotorBDutyCycle[3];    /**< 电机B通道占空比，默认为50% ,(0~255对应0~100%) */
    uint8_t ucDataXOR;               /**< 异或校验：前45位字节异或 */
} S_SpeedMsg_S; 
#pragma pack ()
/* end modified by deng 2023.11.6  */

int32_t SPD_Init(void);

int32_t SPD_SetVelocityConversion(uint32_t ulConversion);

int32_t SPD_SetVelocity(int32_t lVelocityA, int32_t lVelocityB);

void SPD_GetVelocity(int32_t *plVelocityA, int32_t *plVelocityB);

int32_t SPD_SetSpeedFreq(SMB_RecvSpeedMsg_S *pstSpeedMsg);

void SPD_GetCurrentPulse(int32_t *plCurrentPulseA, int32_t *plCurrentPulseB);

#ifdef __cplusplus
}
#endif

#endif /* SPD_H_ */
