/**
 * @brief IO模块（PC-->>SIM/SIM-->>PC）
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
#include "IO.h"
#include "fdcan.h"
#include "Log.h"
#include "Use_Task.h"

/* begin add by deng 2023年11月30日 */
SMB_Queue_S g_astInputIOData; //仿真输入接收队列
STATIC SMB_IoBoardInfo_S g_astInputBoardInfo[SMB_IO_BOARD_NUM]; //输入板io数据缓存区
//STATIC SMB_IoBoardInfo_S g_astOutputBoardInfo[SMB_IO_BOARD_PACK_NUM]; //输出板io数据缓存区
OS_EVENT *g_pstIOMutex = NULL;
/* end add by deng 2023年11月30日 */

// 初始化IO
int32_t IO_Init(void)
{
    uint8_t err = 0;

    //初始化IO
    if (g_pstIOMutex == NULL)
    {
        S_QueueInit(&g_astInputIOData); //输入板io数据队列初始化
        S_MemZero(&g_astInputBoardInfo, SMB_IO_BOARD_PACK_NUM * sizeof(SMB_IoBoardInfo_S)); //输入板io数据缓存区初始化

        g_pstIOMutex = OSMutexCreate(IO_MUTEX_PRIO, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_IO, "IO_Init OSMutexCreate error err(%d)", err);
            return SCP_ERROR;
        }
    }
    else
    {
        OSMutexPend(g_pstIOMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_IO, "IO_Init OSMutexPend error err(%d)", err);
            return SCP_ERROR;
        }

        S_QueueInit(&g_astInputIOData); //输入板io数据队列初始化
        S_MemZero(&g_astInputBoardInfo, SMB_IO_BOARD_PACK_NUM * sizeof(SMB_IoBoardInfo_S)); //输入板io数据缓存区初始化

        err = OSMutexPost(g_pstIOMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_IO, "IO_Init OSMutexPost error err(%d)", err);
            return SCP_ERROR;
        }
    }

    return SCP_OK;
}

// 下发IO数据给IO板
int32_t IO_SendIoInfo(SMB_IoBoardInfo_S astIoBoardInfo[], uint32_t ulBoardNum)
{
    uint32_t ulNum;
    int32_t lRet = 0u;
    SMB_IoBoardInfo_S astOutputBoardInfo[SMB_IO_BOARD_PACK_NUM] = {0}; //输出板io数据缓存区

    if (ulBoardNum > SMB_IO_BOARD_NUM)
    {
        return SCP_ERROR;
    }

    //依次存放各IO单板数据
    for (ulNum = 0; ulNum < ulBoardNum; ulNum++) 
    {
        //判断id是否在规定范围内
        if (astIoBoardInfo[ulNum].ucBoardId <= (uint8_t)IO_OUTPUT_END_CANID && astIoBoardInfo[ulNum].ucBoardId >= (uint8_t)IO_OUTPUT_BEGIN_CANID) //todo: 存在问题，id数据类型由uint8改为16，数据结构没有四字节对齐
        {
            astOutputBoardInfo[ulNum].ucBoardId = astIoBoardInfo[ulNum].ucBoardId;
            astOutputBoardInfo[ulNum].aucData[0] = astIoBoardInfo[ulNum].aucData[0];
            astOutputBoardInfo[ulNum].aucData[1] = astIoBoardInfo[ulNum].aucData[1];
            astOutputBoardInfo[ulNum].aucData[2] = astIoBoardInfo[ulNum].aucData[2];
        }
    }  

    //发送整包IO数据
    lRet = HAL_SendIOMsg(IO_OUTPUT_PACK_CANID, (uint8_t *)&astOutputBoardInfo, SMB_IO_BOARD_PACK_NUM * sizeof(SMB_IoBoardInfo_S));
    if (lRet != SCP_OK)
    {
/* begin  by deng 2024年8月15日 */
        LOG(LOG_ERROR, LOG_MOD_IO, "IO_SendIoInfo error Failed to set IO data.");
        return SCP_ERROR;  // 返回错误，表示无法从硬件驱动中下发IO数据
/* end  by deng 2024年8月15日 */
    }

    return SCP_OK;  
}


// 获取IO数据
int32_t IO_GetIoInfo(SMB_IoBoardInfo_S astIoBoardInfo[], uint32_t *pulBoardNum)
{
/* begin add by deng 2023年11月30日 */
    uint8_t ulID = 0U;//序列号
    uint8_t ulNum = 0U;
    S_Node_S IOData;
    uint8_t err = 0;
/* end add by deng 2023年11月30日 */
/* begin add by deng 2023.11.6 */
    int8_t  cError = 0U; // 接收io数据至主控板卡失败标志位
/* end add by deng 2023.11.6  */

    if (*pulBoardNum < SMB_IO_BOARD_NUM)
    {
        LOG(LOG_ERROR, LOG_MOD_IO, "IO_GetIoInfo error *pulBoardNum(%d)", *pulBoardNum);
        return SCP_ERROR;  // 返回错误
    }

    HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
    S_DeQueue(&g_astInputIOData, &IOData); //从队列取输入板IO数据
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断

    ulID = 0xf & IOData.ulCanSrcId;//取can帧id换算成序列号
    //判断id是否在规定范围内
    if ( ( (uint8_t)IOData.ulCanSrcId <= (uint8_t)IO_INPUT_END_CANID ) && ( (uint8_t)IOData.ulCanSrcId >= (uint8_t)IO_INPUT_BEGIN_CANID)) //todo: 存在问题，id数据类型由uint8改为16，数据结构没有四字节对齐
    {
        OSMutexPend(g_pstIOMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_IO, "IO_Init OSMutexPend error err(%d)", err);
            return SCP_ERROR;
        }
        // 将从硬件驱动中板卡IO数据赋值给传入的指针
        g_astInputBoardInfo[ulID].ucBoardId = (uint8_t)IOData.ulCanSrcId;
        g_astInputBoardInfo[ulID].aucData[0] = IOData.aucData[0];
        g_astInputBoardInfo[ulID].aucData[1] = IOData.aucData[1];
        g_astInputBoardInfo[ulID].aucData[2] = IOData.aucData[2];

        err = OSMutexPost(g_pstIOMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_IO, "IO_Init OSMutexPost error err(%d)", err);
            return SCP_ERROR;
        }
    }
    else
    {
        cError = 1;
        LOG(LOG_ERROR, LOG_MOD_IO, "IO_GetIoInfo ucBoardId(%d) or ulCanSrcId(%d) error", astIoBoardInfo[ulID].ucBoardId, IOData.ulCanSrcId);
    }
    if (cError == 1)
    {
        LOG(LOG_ERROR, LOG_MOD_IO, "IO_GetIoInfo Failed to get IO data.");
        return SCP_ERROR;  // 返回错误，表示无法从硬件驱动中下发IO数据
    }

    //采集io数据上报
    OSMutexPend(g_pstIOMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_IO, "IO_Init OSMutexPend error err(%d)", err);
        return SCP_ERROR;
    }
    for(ulNum=0; ulNum < SMB_IO_BOARD_NUM; ulNum++)
    {
        astIoBoardInfo[ulNum] = g_astInputBoardInfo[ulNum];
    }
    err = OSMutexPost(g_pstIOMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_IO, "IO_Init OSMutexPost error err(%d)", err);
        return SCP_ERROR;
    }

    return SCP_OK;
}
