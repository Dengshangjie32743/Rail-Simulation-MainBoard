/**
 * @brief RADAR：PC-->>SIM
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Zhangmeiyan：首次创建
 *
 */
 
#include "radar.h"     
#include "string.h"
#include "Smb.h"
#include "Log.h"
#include "ucos_ii.h"
#include "usart.h"
#include "Use_Task.h"

int32_t g_lRadarVelocity = 0;
uint32_t g_ulRadarPulseDistance = 0;
uint8_t g_ucRadarSn = 0u;
OS_EVENT *g_pstRadarMutex = NULL;

/*- 雷达数据缓存，一包13字节，10ms一包  */
#define APP_RADAR_DATA_FRAME_SIZE_M         13U         /**< 雷达数据帧长度                               */
#define APP_RADAR_FRAME_HEAD0_M             0x0CU        /**< 帧头0                 */
#define APP_RADAR_FRAME_HEAD1_M             0xFCU        /**< 帧头1                 */

SMB_RecvInjectRadarMsg1_S g_stRadarFi1;
SMB_RecvInjectRadarMsg2_S g_stRadarFi2;
/* begin modified by deng 2023年11月30日 */
//所有字节进行求和，并且对256取模,再用256减去余数
static uint8_t GetSumAndMod256_I(const uint8_t *pData, uint32_t len)
{
    uint32_t uSum = 0;
    uint8_t ucNum = 0;
    for (ucNum = 0; ucNum < len; ucNum++)
    {
        uSum += pData[ucNum]; // 将所有字节相加
    }
/* begin modified by deng 2024年1月30日 */
    return 256 - (uSum % 256); // 返回对256取模，再被256减的结果
/* end modified by deng 2024年1月30日 */
}
/* end modified by deng 2023年11月30日 */

/* begin modified by deng 2023.10.24 */
// 初始化雷达
int32_t RADAR_Init(void)
{
    uint8_t err = 0;

    //初始化雷达
    if (g_pstRadarMutex == NULL)
    {
        g_lRadarVelocity = 0;
        g_ulRadarPulseDistance = 0;
        g_ucRadarSn = 0u;
        memset(&g_stRadarFi1, 0, sizeof(g_stRadarFi1));
        memset(&g_stRadarFi2, 0, sizeof(g_stRadarFi2));

        g_pstRadarMutex = OSMutexCreate(RADAR_MUTEX_PRIO, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_Init OSMutexCreate error err(%d)", err);
            return SCP_ERROR;
        }
    }
    else
    {
        OSMutexPend(g_pstRadarMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_Init OSMutexPend error err(%d)", err);
            return SCP_ERROR;
        }

        g_lRadarVelocity = 0;
        g_ulRadarPulseDistance = 0;
        g_ucRadarSn = 0u;
        memset(&g_stRadarFi1, 0, sizeof(g_stRadarFi1));
        memset(&g_stRadarFi2, 0, sizeof(g_stRadarFi2));

        err = OSMutexPost(g_pstRadarMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_Init OSMutexPost error err(%d)", err);
            return SCP_ERROR;
        }
    }

    return SCP_OK;
}

int32_t RADAR_SetVelocity(int32_t lVelocity, uint32_t ulPulseDistance)
{
    uint8_t err = 0;
    OSMutexPend(g_pstRadarMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SetVelocity OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }

    g_lRadarVelocity = lVelocity;
    g_ulRadarPulseDistance = ulPulseDistance;

    err = OSMutexPost(g_pstRadarMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SetVelocity OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }

    return SCP_OK;
}

uint32_t RADAR_GetPulseDistance(void)
{
    uint32_t ulPulseDistance;
    uint8_t err = 0;

    OSMutexPend(g_pstRadarMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetPulseDistance OSMutexPend error err(%d)", err);
        return 0u;
    }

    ulPulseDistance = g_ulRadarPulseDistance;

    err = OSMutexPost(g_pstRadarMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetPulseDistance OSMutexPost error err(%d)", err);
        return 0u;
    }

    return ulPulseDistance;
}

int32_t RADAR_GetVelocity(void)
{
    int32_t lVelocity = 0;
    uint8_t err = 0;

    OSMutexPend(g_pstRadarMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetPulseDistance OSMutexPend error err(%d)", err);
        return lVelocity;
    }

    lVelocity = g_lRadarVelocity;

    err = OSMutexPost(g_pstRadarMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetPulseDistance OSMutexPost error err(%d)", err);
        return lVelocity;
    }
    return lVelocity;
}

int32_t RADAR_SetInject1(SMB_RecvInjectRadarMsg1_S* pstMsg)
{
    int32_t lRet = SCP_OK;
    uint8_t err = 0;

    OSMutexPend(g_pstRadarMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SetInject1 OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }

    if (pstMsg->ucLen > sizeof(pstMsg->aucData))
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SetInject1 error ucLen(%d)", pstMsg->ucLen);
        lRet = SCP_ERROR;
    }
    else
    {
        g_stRadarFi1 = *pstMsg;
    }

    err = OSMutexPost(g_pstRadarMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SetInject1 OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }

    return lRet;
}

int32_t RADAR_SetInject2(SMB_RecvInjectRadarMsg2_S* pstMsg)
{
    uint8_t err = 0;

    OSMutexPend(g_pstRadarMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SetInject2 OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }

    g_stRadarFi2 = *pstMsg;

    err = OSMutexPost(g_pstRadarMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SetInject2 OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }

    return SCP_OK;
}
/* end modified by deng 2023.10.24 */

uint32_t RADAR_GetRadarMsg(uint8_t aucData[])
{
    AppRadarFrame_ST_M radarData;
    uint32_t ulDist = 0U;
    int32_t lVelocity = 0U;
    uint32_t u32Temp;
    uint8_t status;
    uint8_t err = 0;
    uint8_t ucSn;
    uint32_t ulPulseDistance;

    OSMutexPend(g_pstRadarMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetRadarMsg OSMutexPend error err(%d)", err);
        return 0u;
    }

    lVelocity = g_lRadarVelocity;
    ucSn = g_ucRadarSn;
    g_ucRadarSn++;
    ulPulseDistance = g_ulRadarPulseDistance;

    err = OSMutexPost(g_pstRadarMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetRadarMsg OSMutexPost error err(%d)", err);
        return 0u;
    }

//    ulDist = getulDist(); //todo zzzcccyyy 获取累计脉冲个数函数

    if (lVelocity >= 0)
    {
        status = 0xF6U | 0x01;//前进
    }
    else
    {
        status = 0xF6U & 0xFE;//回退
        lVelocity = -lVelocity;
    }

    radarData.head[0U] = APP_RADAR_FRAME_HEAD0_M;
    radarData.head[1U] = APP_RADAR_FRAME_HEAD1_M;
    radarData.seq = ucSn;
    radarData.v[0u] = (lVelocity >> 8U) & 0xFFU;
    radarData.v[1u] = lVelocity & 0xFFU;

    ulDist = (ulDist * ulPulseDistance) / (1000u * 10u); //todo zzzcccyyy 需要换算单位，换算到cm
    u32Temp = (ulDist / 10U) % 10000U;
    radarData.s1[0u] = (uint8_t) ((u32Temp >> 8U) & 0xFFU);
    radarData.s1[1u] = (uint8_t) (u32Temp & 0xFFU);
    ulDist = ulDist / 100u;                                     //todo zzzcccyyy 需要换算单位，换算到m
    u32Temp = (ulDist / 100u) % 10000U;/*- 0.1km */
    radarData.s2[0u] = (uint8_t) ((u32Temp >> 8U) & 0xFFU);
    radarData.s2[1u] = (uint8_t) (u32Temp & 0xFFU);
    radarData.PWR40 = 0x63U;
    radarData.PWR50 = 0x63U;
    radarData.status = status;
    radarData.fcs = GetSumAndMod256_I((uint8_t *)&radarData, (sizeof(radarData) - 1U));

    memcpy(aucData, &radarData, sizeof(radarData));

    return sizeof(radarData);
}

uint32_t RADAR_InjectField(uint8_t aucData[], uint32_t ulLen, uint32_t ulMaxLen, uint8_t* pucSendCycle)
{
    uint8_t err = 0;
    SMB_RecvInjectRadarMsg1_S stRadarFi1;
    SMB_RecvInjectRadarMsg2_S stRadarFi2;
    uint32_t ulRetLen = ulLen;
    uint8_t i = 0;
    bool_t bIsNeedCrc = SCP_FALSE;

    OSMutexPend(g_pstRadarMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetRadarMsg OSMutexPend error err(%d)", err);
        return 0u;
    }

    stRadarFi1 = g_stRadarFi1;
    *pucSendCycle = g_stRadarFi1.ucSendCycle;
    if (g_stRadarFi1.ucSendCount != 0u)
    {
        g_stRadarFi1.ucSendCount--;
    }
    stRadarFi2 = g_stRadarFi2;

    err = OSMutexPost(g_pstRadarMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_GetRadarMsg OSMutexPost error err(%d)", err);
        return 0u;
    }

    if (stRadarFi1.ucSendCount != 0u)
    {
        if (ulMaxLen >= stRadarFi1.ucLen)
        {
            memcpy(aucData, stRadarFi1.aucData, stRadarFi1.ucLen);
            ulRetLen = stRadarFi1.ucLen;
        }
    }

    for (i = 0; i < (sizeof(stRadarFi2.astField) / sizeof(stRadarFi2.astField[0])); ++i)
    {
        if (stRadarFi2.astField[i].ucEnable != 0u)
        {
            aucData[2 + i] = stRadarFi2.astField[i].ucData;
            bIsNeedCrc = SCP_TRUE;
        }
    }

    if (bIsNeedCrc == SCP_TRUE)
    {
        if (ulRetLen >= 2u)
        {
            /* begin modified by deng 2023年11月30日 */
            aucData[ulRetLen - 1u] = GetSumAndMod256_I(aucData, (ulRetLen - 1U));
            /* end modified by deng 2023年11月30日 */
        }
    }

    return ulRetLen;
}

void RADAR_Loop(void)
{
    INT32U ticks;
    INT32U delay;
    uint8_t aucData[64];
    uint32_t ulLen;
    int32_t lErrorCode = SCP_OK;
    uint8_t ucSendCycle;

    while(1)
    {
        ticks = OSTimeGet();
        ulLen = RADAR_GetRadarMsg(aucData); //获取雷达数据
        if (ulLen != APP_RADAR_DATA_FRAME_SIZE_M)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SendMsg RADAR_GetRadarMsg error ulLen(%d)", ulLen);
            return;
        }

        ulLen = RADAR_InjectField(aucData, ulLen, sizeof(aucData), &ucSendCycle); //雷达报文故障注入（修改报文字段）
        if (ulLen > sizeof(aucData))
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SendMsg RADAR_InjectField error ulLen(%d)", ulLen);
            return;
        }

        lErrorCode = HAL_SendRadarMsg(aucData, ulLen); //发送雷达数据
        if (lErrorCode != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "RADAR_SendMsg HAL_SendRadarMsg error lErrorCode(%d)", lErrorCode);
            return;
        }

        if (ucSendCycle != 0u)
        {
            delay = ucSendCycle - (OSTimeGet() - ticks);    //延时10ms
        }
        else
        {
            delay = 10u - (OSTimeGet() - ticks);    //延时10ms
        }

        if (delay > 3000u)
        {
            delay = 3000u;
        }

        OSTimeDlyHMSM(0,0,0,delay);
    }
}
