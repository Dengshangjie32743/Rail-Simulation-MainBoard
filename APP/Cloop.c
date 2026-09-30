/**
 * @brief 电流环模块（SIM-->>PC）
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie：首次创建
 *
 */
 
#include "Smb.h"
#include "Cloop.h"
#include "adc.h"
#include "Log.h"
#include "udp_socket.h"

uint32_t g_ulClCurrent;
OS_EVENT *g_pstClMutex = NULL;

// 初始化电流环
int32_t CL_Init(void)
{
    uint8_t err = 0;

    if (g_pstClMutex == NULL)
    {
        g_ulClCurrent = 0ul;
        g_pstClMutex = OSMutexCreate(CL_MUTEX_PRIO, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_CL, "CL_Init OSMutexCreate error err(%d)", err);
            return SCP_ERROR;
        }
    }
    else
    {
        OSMutexPend(g_pstClMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_CL, "CL_Loop OSMutexPend error err(%d)", err);
            return SCP_ERROR;
        }

        g_ulClCurrent = 0ul;

        OSMutexPost(g_pstClMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_CL, "CL_Loop OSMutexPost error err(%d)", err);
            return SCP_ERROR;
        }
    }

    return SCP_OK;
}

int32_t CL_GetCLoop(uint32_t* pulCurrent)
{
    uint8_t err = 0;

    if (pulCurrent == SCP_NULL)
    {
        LOG(LOG_ERROR, LOG_MOD_CL, "CL_GetCLoop input error");
        return SCP_ERROR;
    }

    OSMutexPend(g_pstClMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_CL, "CL_Loop OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }

    *pulCurrent = g_ulClCurrent;

    OSMutexPost(g_pstClMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_CL, "CL_Loop OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }

    return SCP_OK;
}

int32_t CL_Loop(void)
{
    uint8_t err = 0;
    uint32_t ulCurrent;

    //从硬件驱动中读取电流环数据(todo： 还需要/65535*3.3v)
    ulCurrent = HAL_GetAdc();
/* begin modified by deng 2024年1月30日 */
    ulCurrent = (ulCurrent * 3.3 * 10000) / 65536 ;//实际值转换为电流值,单位是uA（公式： 电流 = 采集值 * 3.3v * 1000000 / (65536 * 100欧) ）
/* end modified by deng 2024年1月30日 */
//    ulCurrent = 3.3 / 0xffff * ulCurrent;//todo zzzcccyyy 需要从AD单位换算到电流mA的单位

    // 将从硬件驱动中读取的电流环给传入的指针
    OSMutexPend(g_pstClMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_CL, "CL_Loop OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }

    g_ulClCurrent = ulCurrent;

    OSMutexPost(g_pstClMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_CL, "CL_Loop OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }

    return SCP_OK;  // 返回成功，表示已经成功从硬件驱动中读取电流环数据
}

