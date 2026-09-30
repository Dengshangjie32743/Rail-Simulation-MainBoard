#include "Use_Task.h"
#include "udp_socket.h"
#include "btm.h"
/* begin modified by deng 2023.10.24 */
//#include "rs485.h"
#include "usart.h"
#include "simPro.h"
/* end modified by deng 2023.10.24 */
#include "radar.h"
#include "fdcan.h"
#include "stm32h7xx_hal_fdcan.h"
#include "Log.h"
#include "PWM.h"
#include "Cloop.h"
/* begin modified by deng 2023年11月20日 */
#include "led.h"
/* end modified by deng 2023年11月20日 */
/* begin add by deng 2023年11月25日 */
#include "lan8720.h"
/* end add by deng 2023年11月25日 */
#include "CPUusage.h"
#include "lwip_comm.h"
/* begin add by deng 2024年12月1日 */
#include "lwip/sockets.h"
#include "lwip_comm.h"
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
/* end add by deng 2024年12月1日 */

typedef struct 
{
    uint32_t ulCpuUsagePrio[64]; //cpu运行时间优先级
    uint32_t ulCpuUsageTime; //cpu运行时间长度
    uint32_t aulCpuUsage[64]; //任务占用时间长度
}SMB_CpuUsage_S;
extern SMB_CpuUsage_S g_stScpCpuUsage;

uint8_t g_aucRecvBuf[UDP_BUFFSIZE];
void RECV_Task(void)
{
    uint16_t usLen = 0;
    int32_t lRet;

    SOCKET_ADDR sock_addr;
    /* begin add by deng 2025年3月1日 用于调试*/
    struct in_addr addr;
    /* end add by deng 2025年3月1日 */
    usLen = fn_RecvFrom_RXQueue(g_aucRecvBuf, sizeof(g_aucRecvBuf), &sock_addr, 0, Udp_Recv_Buf_Mutex);
    if (usLen > 0)
    {
        LOG_BIN(LOG_DEBUG, LOG_MOD_MAIN, g_aucRecvBuf, (uint32_t)usLen, "RECV_Task fn_RecvFrom_RXQueue");

        lRet = SP_Process(g_aucRecvBuf, (uint32_t)usLen, sock_addr);
        if (lRet != SCP_OK)
        {
            /* begin add by deng 2025年3月1日 用于调试*/
//            addr.s_addr = htonl(sock_addr.ui_SinAddr); // 确保转换为网络字节序
//            LOG(LOG_ERROR, LOG_MOD_MAIN, "RECV_Task sock_addr SP_Process error NET_Addr(%s) NET_Port(%d)", inet_ntoa(addr), sock_addr.us_SinPort);
            /* end add by deng 2025年3月1日 */
            LOG_BIN(LOG_ERROR, LOG_MOD_MAIN, g_aucRecvBuf, (uint32_t)usLen, "RECV_Task SP_Process error lRet(%d)", lRet);
        }
    }
}

/* begin modified by deng 2023.10.27 */
/*
 * 雷达任务
 */
void Radar_Task(void *arg)
{
    RADAR_Loop();

    LOG(LOG_ERROR, LOG_MOD_MAIN, "RECV_Task RADAR_Loop error");
}   

/*
 * 采集任务
 */
void Collect_Task(void *arg)
{
    int32_t lRet = SCP_OK;

    while(1)
    {

        lRet = PWM_Loop();
        if (lRet != SCP_OK)
        {
            LOG(LOG_DEBUG, LOG_MOD_MAIN, "Collect_Task PWM_Loop error lRet(%d)", lRet);
        }


        lRet = CL_Loop();
        if (lRet != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_MAIN, "Collect_Task CL_Loop error lRet(%d)", lRet);
        }

        OSTimeDlyHMSM(0,0,0,500);  //延时500ms（时间短影响性能）
    }
} 

/*
 * 开关量IO任务，方向:SIM<->PC 
 */
void IO_Task(void *arg)
{
    while(1)
    {
        OSTimeDlyHMSM(0,0,0,1000);  //延时1000ms
    }
}         

void BTM1_Task(void *arg)
{
    int32_t lRet;
    while(1)
    {

        lRet = BTM_Loop(0);
        if (lRet != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_MAIN, "BTM1_Task BTM_Loop error lRet(%d)", lRet);
        }
        OSTimeDlyHMSM(0,0,0,5);  //延时5ms
    }
}   

void BTM2_Task(void *arg)
{
    int32_t lRet;
    while(1)
    {
        lRet = BTM_Loop(1);
        if (lRet != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_MAIN, "BTM2_Task BTM_Loop error lRet(%d)", lRet);
        }

        OSTimeDlyHMSM(0,0,0,5);  //延时5ms
    }
}

/*
 * 速度任务，方向:PC->SIM 
 */
void VELOCITY_Task(void *arg)
{
    OSTimeDlyHMSM(0,0,0,100);  //延时100ms
}   

/*
 * SP周期发送任务
 */
void SP_Task(void *arg)
{
    SP_Loop();
}   
/* end modified by deng 2023.10.27 */

/* begin modified by deng 2023年11月20日 */
/*
 * led任务和网络初始化任务
 */
void Led_Task(void *pdata)
{
    while (lwip_comm_init()) //lwip初始化
    {
        LOG(LOG_ERROR, LOG_MOD_Init, "Lwip Init failed!");
    }

    while (1)
    {
        LED0_Toggle; //运行灯

        //判断网络连接状态是否正常
        if(LAN8720_GetLinkState() <= LAN8720_STATUS_LINK_DOWN)
        {
            LED1(1);
        }
        else
        {
            LED1(0);
        }

        OSTimeDlyHMSM(0, 0, 0, 50);  //延时50ms
    }
}
/* end modified by deng 2023年11月20日 */

/*  start add by deng 2024年8月15日*/
/*
 * CPU占用率统计任务
 */
void CpuUsage_Task(void* pdata)
{
    CpuUsage_LOOP();
}

/*
 * 日志任务
 */
void Log_Task(void *arg)
{
    SOCKET_ADDR stSPSockaddr;

    stSPSockaddr.us_SinFamliy = AF_INET;// IPv4 网络协议
    stSPSockaddr.ui_SinAddr = 0xc0a801F9;//IP:192.168.1.249
    stSPSockaddr.us_SinPort = 8089;//端口号

    LOG_LOOP(stSPSockaddr);
}
/*  end add by deng 2024年8月15日*/

