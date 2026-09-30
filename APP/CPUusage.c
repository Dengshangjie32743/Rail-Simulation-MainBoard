/**
 * @brief CPU占用率模块
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2024-08-15   Dengshangjie：首次创建
 *
 */

#include "Smb.h"
#include "Log.h"
#include "Use_Task.h"

typedef struct 
{
    uint32_t ulCpuUsagePrio[64]; //cpu运行时间优先级
    uint32_t ulCpuUsageTime; //cpu运行时间长度
    uint32_t aulCpuUsage[64]; //任务占用时间长度
}SMB_CpuUsage_S;

SMB_CpuUsage_S g_stScpCpuUsage = {0};

//操作系统调度钩子函数
void CpuUsageHook(void)
{
//    uint32_t ulTick = OSTime;//当前系统时间
//    static uint32_t ulLastTick1 = 0u;
//    static uint32_t ulLastTick = 0u;
//    static uint32_t aulSum[64] = {0};
//    uint32_t ulNum = 0U;
//    uint32_t ulUsed = 0U;
//
//    ulUsed = ulTick - ulLastTick1;
//    ulLastTick1 = ulTick;
//    aulSum[OSTCBCur->OSTCBPrio] += ulUsed;
//
//    //每隔一秒钟记录一次
//    if ((ulTick - ulLastTick) >= 1000u)
//    {
//        for(ulNum = 0; ulNum <= OS_TASK_IDLE_PRIO; ulNum++)//每个任务的占用率（即计算任务运行时间）
//        {
//            g_stScpCpuUsage.ulCpuUsagePrio[ulNum] = ulNum;
//            g_stScpCpuUsage.ulCpuUsageTime = ulTick - ulLastTick;
//            g_stScpCpuUsage.aulCpuUsage[ulNum] = aulSum[ulNum];//占用时间比例
//            aulSum[ulNum] = 0u;
//        }
//        ulLastTick = ulTick;
//    }
}

void CpuUsage_LOOP(void)
{
    uint8_t ulNum;
    STATIC SMB_CpuUsage_S stScpCpuUsage;
    int i;
    OS_CPU_SR cpu_sr;

    while(1)
    {
        LOG(LOG_FATAL, LOG_MOD_CPU_USAGE, "*********************  App Task Usage *********************");
        LOG(LOG_FATAL, LOG_MOD_CPU_USAGE, "Prio   Time   TotalTime");
        OS_ENTER_CRITICAL();      //关中断
        for (i = 0; i < 64u; i++)
        {
            stScpCpuUsage.aulCpuUsage[i] = g_stScpCpuUsage.aulCpuUsage[i];
            stScpCpuUsage.ulCpuUsagePrio[i] = g_stScpCpuUsage.ulCpuUsagePrio[i];
        }
        stScpCpuUsage.ulCpuUsageTime = g_stScpCpuUsage.ulCpuUsageTime;
        OS_EXIT_CRITICAL();          //开中断

        for(ulNum = 0; ulNum < 64u; ulNum++)//每个任务的占用率（运行时间长度）
        {
           if(stScpCpuUsage.aulCpuUsage[ulNum] != 0u)
           {
              LOG(LOG_FATAL, LOG_MOD_CPU_USAGE,"%d     %d    %d", stScpCpuUsage.ulCpuUsagePrio[ulNum], stScpCpuUsage.aulCpuUsage[ulNum],stScpCpuUsage.ulCpuUsageTime);//打印优先级和占用时间
              stScpCpuUsage.aulCpuUsage[ulNum] = 0u;
           }

        }
        OSTimeDlyHMSM(0, 0, 1, 0);
    }
}




