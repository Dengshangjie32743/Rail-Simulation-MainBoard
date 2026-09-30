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
#include "led.h"
#include "TypeInc.h"
#include "Smb.h"
#include "PWM.h"
#include "spi.h"
#include "Log.h"
#include "Slib.h"
#include "Use_Task.h"
uint32_t g_ulPwmFreq;
uint32_t g_ulPwmDutyCycle;
/* begin add by deng 2023年11月30日 */
extern int32_t g_lCurrentPulseA; /**< 速度计A累计脉冲数 */
extern int32_t g_lCurrentPulseB; /**< 速度计B累计脉冲数 */
/* end add by deng 2023年11月30日 */
OS_EVENT *g_pstPwmMutex = NULL;

// 初始化PWM
int32_t PWM_Init(void)
{
    uint8_t err = 0;

    if (g_pstPwmMutex == NULL)
    {
        g_ulPwmFreq = 0ul;
        g_ulPwmDutyCycle = 0ul;
        g_pstPwmMutex = OSMutexCreate(PWM_MUTEX_PRIO, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_Init OSMutexCreate error err(%d)", err);
            return SCP_ERROR;
        }
    }
    else
    {
        OSMutexPend(g_pstPwmMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_Init OSMutexPend error err(%d)", err);
            return SCP_ERROR;
        }

        g_ulPwmFreq = 0ul;
        g_ulPwmDutyCycle = 0ul;

        OSMutexPost(g_pstPwmMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_Init OSMutexPost error err(%d)", err);
            return SCP_ERROR;
        }
    }

    return SCP_OK;
}

// 获取PWM数据
int32_t PWM_GetPwm(uint32_t *pulPwmFreq, uint32_t *pulPwmDutyCycle)
{  
    uint8_t err = 0;

    if ((pulPwmFreq == SCP_NULL) || (pulPwmDutyCycle == SCP_NULL))
    {
        LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_GetPwm input error");
        return SCP_ERROR;
    }
  
    OSMutexPend(g_pstPwmMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_GetPwm OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }

    *pulPwmFreq = g_ulPwmFreq;
    *pulPwmDutyCycle = g_ulPwmDutyCycle;

    OSMutexPost(g_pstPwmMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_GetPwm OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }
  
    return SCP_OK;  // 返回成功，表示已经成功从硬件驱动中读取PWM数据  
}

int32_t PWM_Loop(void)
{
    uint8_t err = 0;
/* begin modified by deng 2023.11.6 */
    S_PWMMsg_S stPwmDutyCycle = {0};
    uint16_t usPwmHighLevel = 0;
    uint16_t usPwmLowLevel = 0;
    uint32_t ulPwmDutyCycle = 0;
/* begin add by deng 2023年11月30日 */
    int32_t lAccumulatedPulseA;
    int32_t lAccumulatedPulseB;
/* end add by deng 2023年11月30日 */
    int32_t lErrorCode = SCP_OK;

    //从硬件驱动中读取占空比（todo：还需要读取频率，但是spi协议里没有频率值）
    lErrorCode = HAL_ReceivePWM((uint8_t *)&stPwmDutyCycle, sizeof(S_PWMMsg_S));
    if (lErrorCode != SCP_OK)
    {
        LOG(LOG_WARNING, LOG_MOD_PWM, "PWM_GetPwm Failed to read PWM data");
        lErrorCode = SCP_ERROR;
    }
/* begin add by deng 2025年3月1日 打印spi接收原始数据， 用于调试*/
//    LOG_BIN(LOG_ERROR, LOG_MOD_PWM, (const uint8_t*)&stPwmDutyCycle, sizeof(S_PWMMsg_S), "PWM_Loop SPI error");
/* end add by deng 2025年3月1日 */

    //判断采集的pwm报文是否正确
    if (stPwmDutyCycle.ucPWMHeadA != 0x53 || stPwmDutyCycle.ucPWMHeadB != 0x47 || stPwmDutyCycle.ucPWMTail != 0x0f)
    {
        LOG_BIN(LOG_ERROR, LOG_MOD_PWM, (const uint8_t*)&stPwmDutyCycle, sizeof(S_PWMMsg_S), "PWM_Loop stPwmDutyCycle error");
        LOG(LOG_WARNING, LOG_MOD_PWM, "PWM_GetPwm error data HeadA(%d) or HeadB(%d) or Tail(%d)", stPwmDutyCycle.ucPWMHeadA, stPwmDutyCycle.ucPWMHeadB, stPwmDutyCycle.ucPWMTail);
        lErrorCode = SCP_ERROR;  // 返回错误，表示回复帧头错误
    }
/* begin modified by deng 2025年3月1日 */
//    if (stPwmDutyCycle.ucDataXOR != S_CheckXOR((uint8_t *)&stPwmDutyCycle, sizeof(S_PWMMsg_S) - 21U))
    if (stPwmDutyCycle.ucDataXOR != S_CheckXOR((uint8_t *)&stPwmDutyCycle, sizeof(S_PWMMsg_S) - 27U))
/* end modified by deng 2025年3月1日 */
    {
        LOG_BIN(LOG_ERROR, LOG_MOD_PWM, (const uint8_t*)&stPwmDutyCycle, sizeof(S_PWMMsg_S), "PWM_Loop stPwmDutyCycle error");
        LOG(LOG_WARNING, LOG_MOD_PWM, "PWM_GetPwm error PWM data XOR(%d) != self XOR(%d)", stPwmDutyCycle.ucDataXOR, S_CheckXOR((uint8_t *)&stPwmDutyCycle, sizeof(S_PWMMsg_S) - 21U));
        lErrorCode = SCP_ERROR;  // 返回错误，表示回复异或值错误
    }
/* end modified by deng 2023.11.6  */
/* begin add by deng 2023年11月30日 */
    usPwmHighLevel = (stPwmDutyCycle.ucHighLevelTimeHighA << 8) | stPwmDutyCycle.ucHighLevelTimeLowA;//PWM_A高电平采集点数
    usPwmLowLevel  = (stPwmDutyCycle.ucLowLevelTimeHighA << 8) | stPwmDutyCycle.ucLowLevelTimeLowA;  //PWM_A低电平采集点数
    //判断占空比计算公式里的被除数是否为0
    if ((usPwmHighLevel + usPwmLowLevel) != 0)
    {
        ulPwmDutyCycle = (usPwmHighLevel * 100u) / (usPwmHighLevel + usPwmLowLevel);  //速度计A占空比（ 公式：100*高电平采集点数/(高电平采集点数+低电平采集点数) ）
    }
    else
    {
        ulPwmDutyCycle = 0xffffffff; //若占空比被除数为0,返回0xffffffff
    }
    lAccumulatedPulseA = (stPwmDutyCycle.ucAccumulatedPulseHighestA << 24) |(stPwmDutyCycle.ucAccumulatedPulseHighA << 16) \
                             | (stPwmDutyCycle.ucAccumulatedPulseLowA << 8) | stPwmDutyCycle.ucAccumulatedPulseLowestA;      //PWM_A累计脉冲
    lAccumulatedPulseB = (stPwmDutyCycle.ucAccumulatedPulseHighestB << 24) |(stPwmDutyCycle.ucAccumulatedPulseHighB << 16) \
                             | (stPwmDutyCycle.ucAccumulatedPulseLowB << 8) | stPwmDutyCycle.ucAccumulatedPulseLowestB;      //PWM_B累计脉冲
/* end add by deng 2023年11月30日 */
    // 将从硬件驱动中读取的PWM频率和占空比赋值给传入的指针
    OSMutexPend(g_pstPwmMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_Loop OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }
/* begin modified by deng 2023.11.6 */
    if ((usPwmHighLevel + usPwmLowLevel) != 0)
    {
        g_ulPwmFreq = PWM_FREQ_TOTAL / (usPwmHighLevel + usPwmLowLevel);//PWM_A频率
    }
    else
    {
        g_ulPwmFreq = 0u;//PWM_A频率
    }
    g_ulPwmDutyCycle = ulPwmDutyCycle; //todo:spi可以获取两路占空比，暂时使用第一路，第二路空闲
    g_lCurrentPulseA = lAccumulatedPulseA;//PWM_A累计脉冲
    g_lCurrentPulseB = lAccumulatedPulseB;//PWM_B累计脉冲
/* end modified by deng 2023.11.6  */

    OSMutexPost(g_pstPwmMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_PWM, "PWM_Loop OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }

    return lErrorCode;  // 返回成功，表示已经成功从硬件驱动中读取PWM数据
}
