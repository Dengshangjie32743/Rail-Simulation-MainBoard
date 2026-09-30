/**
 * @brief PWM模块（SIM-->>PC）
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie：首次创建
 *
 */
 
#ifndef PWM_H_
#define PWM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Smb.h"

/* begin add by deng 2023年11月30日 */
#define PWM_FREQ_TOTAL 60000 //PWM频率总值为60KHz
/* end add by deng 2023年11月30日 */

/* begin modified by deng 2023.11.6 */
/**
 * @brief PWM数据结构体
 */
typedef struct {
    uint8_t ucPWMHeadA;   /**< 报文头1: 0x53 */
    uint8_t ucPWMHeadB;   /**< 报文头2: 0x47 */
    uint8_t ucHighLevelTimeHighA;     /**< 第1路PWM高电平采集点数1 （1表示高8位，2表示低8位）*/
    uint8_t ucHighLevelTimeLowA;      /**< 第1路PWM高电平采集点数2 （1表示高8位，2表示低8位）*/
    uint8_t ucLowLevelTimeHighA;      /**< 第1路PWM低电平采集点数1 （1表示高8位，2表示低8位）*/
    uint8_t ucLowLevelTimeLowA;       /**< 第1路PWM低电平采集点数2 （1表示高8位，2表示低8位）*/
    uint8_t ucHighLevelTimeHighB;     /**< 第2路PWM高电平采集点数1 （1表示高8位，2表示低8位）*/
    uint8_t ucHighLevelTimeLowB;      /**< 第2路PWM高电平采集点数2 （1表示高8位，2表示低8位）*/
    uint8_t ucLowLevelTimeHighB;      /**< 第2路PWM低电平采集点数1 （1表示高8位，2表示低8位）*/
    uint8_t ucLowLevelTimeLowB;       /**< 第2路PWM低电平采集点数2 （1表示高8位，2表示低8位）*/
/* begin add by deng 2024年1月30日 */
    uint8_t ucAccumulatedPulseHighestA;  /**< 第1路PWM累计脉冲值1 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
    uint8_t ucAccumulatedPulseHighA;     /**< 第1路PWM累计脉冲值3 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
    uint8_t ucAccumulatedPulseLowA;      /**< 第1路PWM累计脉冲值2 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
    uint8_t ucAccumulatedPulseLowestA;   /**< 第1路PWM累计脉冲值4 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
    uint8_t ucAccumulatedPulseHighestB;  /**< 第2路PWM累计脉冲值1 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
    uint8_t ucAccumulatedPulseHighB;     /**< 第2路PWM累计脉冲值3 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
    uint8_t ucAccumulatedPulseLowB;      /**< 第2路PWM累计脉冲值2 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
    uint8_t ucAccumulatedPulseLowestB;   /**< 第2路PWM累计脉冲值4 （1表示最高8位，2表示高8位,3表示低8位，4表示最低8位）*/
/* end add by deng 2024年1月30日 */
    uint8_t ucPWMTail;    /**< 报文尾: 0x0F */
    uint8_t ucDataXOR;    /**< 异或校验：前7位字节异或 */
    uint8_t ucResv[26];   /**< 预留位 */
} S_PWMMsg_S;
/* end modified by deng 2023.11.6  */

// 初始化PWM
int32_t PWM_Init(void);
// 获取PWM数据
int32_t PWM_GetPwm(uint32_t *pulPwmFreq, uint32_t *pulPwmDutyCycle);
int32_t PWM_Loop(void);

#ifdef __cplusplus
}
#endif

#endif /* PWM_H_ */
