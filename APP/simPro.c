/**
 * @brief 板间通信协议层
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file   simPro.c
 * @ingroup
 * @version V1.0.0
 *
 * @date 2023-10-01   朱成寅：首次创建
 *
 *
 */
#include "TypeInc.h"
/* begin modified by deng 2023.10.24 */
//#include <winsock.h>
#include "udp_socket.h"
/* end modified by deng 2023.10.24 */
#include "Smb.h"
#include "Cloop.h"
#include "Radar.h"
#include "Speed.h"
#include "PWM.h"
#include "BTM.h"
#include "simPro.h"
#include "IO.h"
#include "Log.h"
#include "udp_socket.h"

uint32_t g_ulSPCycle; //上报周期
uint32_t g_ulSPPulseDistance;
SOCKET_ADDR g_stSPSockaddr;
OS_EVENT *g_pstSimProMutex = NULL;

// 协议初始化
void SP_Init(void)
{
    uint8_t err = 0;

    if (g_pstSimProMutex == NULL)
    {
        g_ulSPCycle = 0u;
        g_ulSPPulseDistance = 0u;
        memset(&g_stSPSockaddr, 0, sizeof(g_stSPSockaddr));

        g_pstSimProMutex = OSMutexCreate(SP_MUTEX_PRIO, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexCreate error err(%d)", err);
            return;
        }
    }
    else
    {
        OSMutexPend(g_pstSimProMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPend error err(%d)", err);
            return;
        }

        g_ulSPCycle = 0u;
        g_ulSPPulseDistance = 0u;
        memset(&g_stSPSockaddr, 0, sizeof(g_stSPSockaddr));

        OSMutexPost(g_pstSimProMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPost error err(%d)", err);
            return;
        }
    }

    return;
}

// SMB初始化
STATIC void SP_InitSmb(void)
{
    int32_t lRet = SCP_OK;

    // BTM初始化
    lRet = BTM_CleanBtmData();
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_InitSmb error BTM_CleanBtmData(%d)", lRet);
    }

    // io初始化
    lRet = IO_Init();
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_InitSmb error IO_Init(%d)", lRet);
    }

    // 雷达初始化
    lRet = RADAR_Init();
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_InitSmb error RADAR_Init(%d)", lRet);
    }

    // 速度计初始化
    lRet = SPD_Init();
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_InitSmb error SPD_Init(%d)", lRet);
    }
}

// 数据查询,回送对应类型的状态数据
STATIC void SP_AskSmb(uint16_t usMsgType, SOCKET_ADDR stSockaddr)
{
    int32_t lRet = SCP_OK;
    uint32_t ulBoardNum;
    uint32_t ulDatalen;
    SMB_IoMsg_S stIoMsg = {0};
    SMB_SendCloopPwmMsg_S stCloopPwmMsg = {0};

    switch (usMsgType)
    {
        //获取电流环和pwm数据
        case SMB_SEND_MSG_TYPE_CLOOP_PWM:
        {
            //获取电流环数据
            lRet = CL_GetCLoop(&stCloopPwmMsg.ulCurrentCLoop);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_AskSmb error CL_GetCLoop(%d)", lRet);
                return;
            }

            //获取pwm数据
            lRet = PWM_GetPwm(&stCloopPwmMsg.ulPwmFreq, &stCloopPwmMsg.ulPwmDutyCycle);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_AskSmb error PWM_GetPwm(%d)", lRet);
                return;
            }

            stCloopPwmMsg.stMsgHead.ucVersion = SMB_INTERFACE_VERSION;
            stCloopPwmMsg.stMsgHead.usMsgType = htons(SMB_SEND_MSG_TYPE_CLOOP_PWM);
            stCloopPwmMsg.ulCurrentCLoop = htonl(stCloopPwmMsg.ulCurrentCLoop);
            stCloopPwmMsg.ulPwmFreq = htonl(stCloopPwmMsg.ulPwmFreq);
            stCloopPwmMsg.ulPwmDutyCycle = htonl(stCloopPwmMsg.ulPwmDutyCycle);
            // lRet = UDP_Send(&stCloopPwmMsg, sizeof(stCloopPwmMsg));
            lRet = fn_Sendto_TXQueue((uint8_t *)&stCloopPwmMsg, sizeof(stCloopPwmMsg), &stSockaddr, 0, Udp_Recv_Buf_Mutex);
            if (lRet != SYS_YES)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_AskSmb error usMsgType(0x%04x) UDP_Send(%d)", usMsgType, lRet);
                return;
            }

            break;
        }

        //查询IO数据
        case SMB_SEND_MSG_TYPE_IO:
        {
            stIoMsg.stMsgHead.ucVersion = SMB_INTERFACE_VERSION;
            stIoMsg.stMsgHead.usMsgType = htons(SMB_SEND_MSG_TYPE_IO);
            ulBoardNum = SMB_IO_BOARD_NUM;

            //获取IO数据
            lRet = IO_GetIoInfo(stIoMsg.astIoBoardInfo, &ulBoardNum);
            //判断IO板卡数量是否超出上限
            if ((lRet != SCP_OK) || (ulBoardNum > SMB_IO_BOARD_NUM))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_AskSmb error IO_GetIoInfo(%d) ulBoardNum(%d)", lRet, ulBoardNum);
                return;
            }
            ulDatalen = sizeof(stIoMsg.stMsgHead) + (sizeof(stIoMsg.astIoBoardInfo[0]) * ulBoardNum); //数据长度是52字节

            //lRet = UDP_Send(&stIoMsg, ulDatalen);
            lRet = fn_Sendto_TXQueue((uint8_t *) &stIoMsg, ulDatalen, &stSockaddr, 0, Udp_Recv_Buf_Mutex);
            if (lRet != SYS_YES)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_AskSmb error usMsgType(0x%04x) UDP_Send(%d)", usMsgType, lRet);
                return;
            }
            break;
        }

        //查询BTM数据(todo:BTM协议待定)
        case SMB_SEND_MSG_TYPE_BTM:
        {
            break;
        }
        default:
        {
            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_AskSmb error usMsgType(0x%04x)", usMsgType);
        }
    }
}

// 数据的接收与下发
int32_t SP_Process(uint8_t *pucData, uint32_t ulDataLen, SOCKET_ADDR stSockaddr)
{
    int32_t lRet = SCP_OK;
    uint32_t i;
    SMB_MsgHead_S* pstMsgHead;
    SMB_InitMsg_S* pstInitMsg;
    SMB_RecvAskMsg_S* pstAskMsg;
    SMB_IoMsg_S* pstIoMsg;
    uint32_t ulBoardNum = 0u;
    SMB_RecvVelocityMsg_S* pstVelocityMsg;
    SMB_RecvSpeedMsg_S* pstSpeedMsg;
    uint32_t ulSPPulseDistance;
    uint8_t err = 0;

TRY_BEGIN
    /*- 入参判断 */
    if ((SCP_NULL == pucData) || (ulDataLen < sizeof(SMB_MsgHead_S)))
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error pucData(%p) ulDataLen(%d)", pucData, ulDataLen);
        THROW(SCP_ERROR);
    }

    pstMsgHead = (SMB_MsgHead_S*)pucData;

    if (pstMsgHead->ucVersion != SMB_INTERFACE_VERSION)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error MSG VERSION(0x%02x) SMB_INTERFACE_VERSION(%d)", pstMsgHead->ucVersion, SMB_INTERFACE_VERSION);
        THROW(SCP_ERROR);
    }

    switch (ntohs(pstMsgHead->usMsgType))
    {
        //初始化报文
        case SMB_RECV_MSG_TYPE_INIT:
        {
            if (ulDataLen != sizeof(SMB_InitMsg_S))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_RECV_MSG_TYPE_INIT ulDataLen(%d)", ulDataLen);
                THROW(SCP_ERROR);
            }

            pstInitMsg = (SMB_InitMsg_S*)pucData;
            SP_InitSmb();
            OSMutexPend(g_pstSimProMutex, 0, &err);
            if (err != OS_ERR_NONE)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPend error err(%d)", err);
                THROW(SCP_ERROR);
            }

            g_ulSPCycle = ntohl(pstInitMsg->ulCycle);
            g_stSPSockaddr = stSockaddr;
            g_ulSPPulseDistance = ntohl(pstInitMsg->ulPulseDistance);
//            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process INIT  Cycle(%d)  Conver(%d)", g_ulSPCycle, ntohl(pstInitMsg->ulConversion));

            OSMutexPost(g_pstSimProMutex);
            if (err != OS_ERR_NONE)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPost error err(%d)", err);
                THROW(SCP_ERROR);
            }
            lRet = SPD_SetVelocityConversion(ntohl(pstInitMsg->ulConversion));
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SPD_SetVelocityConversion(%d) ulConversion(%d)", lRet, ntohl(pstInitMsg->ulConversion));
                THROW(SCP_ERROR);
            }
/* begin add by deng 2023年11月30日*/
            else if (lRet == SCP_OK)
            {
                LOG(LOG_INFO, LOG_MOD_PRO, "SP_Process Init OK!");
            }
/* end add by deng 2023年11月30日 */
            break;
        }
        //数据查询报文
        case SMB_RECV_MSG_TYPE_ASK:
        {
            if (ulDataLen != sizeof(SMB_RecvAskMsg_S))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_RECV_MSG_TYPE_ASK ulDataLen(%d)", ulDataLen);
                THROW(SCP_ERROR);
            }
            pstAskMsg = (SMB_RecvAskMsg_S*)pucData;
            SP_AskSmb(ntohs(pstAskMsg->usAskMsgType), stSockaddr);
            break;
        }
        //开关量IO输出报文
        case SMB_RECV_MSG_TYPE_IO:
        {
            if (((ulDataLen - sizeof(SMB_MsgHead_S)) % sizeof(SMB_IoBoardInfo_S)) != 0u)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_RECV_MSG_TYPE_IO usMsgType(0x%04x) ulDataLen(%d)", ntohs(pstMsgHead->usMsgType), ulDataLen);
                THROW(SCP_ERROR);
            }

            pstIoMsg = (SMB_IoMsg_S*)pucData;
            ulBoardNum = (ulDataLen - sizeof(SMB_MsgHead_S)) / sizeof(SMB_IoBoardInfo_S);
            lRet = IO_SendIoInfo(pstIoMsg->astIoBoardInfo, ulBoardNum);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error IO_SendIoInfo(%d)", lRet);
                THROW(SCP_ERROR);
            }
/* begin add by deng 2023年11月30日 */
            else if (lRet == SCP_OK)
            {
                LOG(LOG_INFO, LOG_MOD_IO, "SP_Process IO_SendIoInfo OK!");
            }
/* end add by deng 2023年11月30日 */
            break;
        }

        //速度计报文
        case SMB_RECV_MSG_TYPE_SEPPD:
        {
            if (ulDataLen != sizeof(SMB_RecvSpeedMsg_S))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_RECV_MSG_TYPE_SEPPD ulDataLen(%d)", ulDataLen);
                THROW(SCP_ERROR);
            }

            pstSpeedMsg = (SMB_RecvSpeedMsg_S*)pucData;
            for (i = 0u; i < (sizeof(pstSpeedMsg->stSpeedA) / sizeof(pstSpeedMsg->stSpeedA[0])); ++i)
            {
                pstSpeedMsg->stSpeedA[i].usFreq = ntohs(pstSpeedMsg->stSpeedA[i].usFreq);
                pstSpeedMsg->stSpeedA[i].usCount = ntohs(pstSpeedMsg->stSpeedA[i].usCount);
                pstSpeedMsg->stSpeedA[i].sPhase = ntohs(pstSpeedMsg->stSpeedA[i].sPhase);
                pstSpeedMsg->stSpeedB[i].usFreq = ntohs(pstSpeedMsg->stSpeedB[i].usFreq);
                pstSpeedMsg->stSpeedB[i].usCount = ntohs(pstSpeedMsg->stSpeedB[i].usCount);
                pstSpeedMsg->stSpeedB[i].sPhase = ntohs(pstSpeedMsg->stSpeedB[i].sPhase);
            }
            lRet = SPD_SetSpeedFreq(pstSpeedMsg);
            if (lRet != SCP_OK)
            {
/* begin modified by deng 2023年11月30日 */
//                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SPD_SetSpeedFreq(%d)", lRet);
                LOG(LOG_ERROR, LOG_MOD_SPEED, "SP_Process error SPD_SetSpeedFreq(%d)", lRet);
/* end modified by deng 2023年11月30日 */
                THROW(SCP_ERROR);
            }
/* begin add by deng 2023年11月30日 */
            else if (lRet == SCP_OK)
            {
                LOG(LOG_INFO, LOG_MOD_SPEED, "SP_Process SPD_SetSpeedFreq OK!");
            }
/* end add by deng 2023年11月30日 */
            break;
        }

        //BTM报文
        case SMB_RECV_MSG_TYPE_BTM:
        {
            if (ulDataLen != sizeof(SMB_BtmMsg_S))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_RECV_MSG_TYPE_BTM ulDataLen(%d)", ulDataLen);
                THROW(SCP_ERROR);
            }
//            /* begin add by deng 2025年3月1日 用于调试*/
//            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_RECV_MSG_TYPE_BTM ulDataLen(%d)", ulDataLen);
//            /* end add by deng 2025年3月1日 */
            lRet = BTM_SetBtmData((SMB_BtmMsg_S*)pucData);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error BTM_SetBtmData(%d)", lRet);
                THROW(SCP_ERROR);
            }
/* begin add by deng 2023年11月30日 */
            else if (lRet == SCP_OK)
            {
                LOG(LOG_INFO, LOG_MOD_SPEED, "SP_Process BTM_SetBtmData OK!");
            }
/* end add by deng 2023年11月30日 */
            break;
        }

/* begin add by deng 2024年1月30日 */
        //BTM故障注入报文1
        case SMB_INJECT_MSG_TYPE_BTM1:
        {

            lRet = BTM_RecvInjectBTMMsg1((SMB_RecvInjectBTMMsg1_S*)pucData);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error BTM_SetBtmData(%d)", lRet);
                THROW(SCP_ERROR);
            }
            break;
        }
/* end add by deng 2024年1月30日 */

        //速度报文
        case SMB_RECV_MSG_TYPE_VELOCITY:
        {
            if (ulDataLen != sizeof(SMB_RecvVelocityMsg_S))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_RECV_MSG_TYPE_VELOCITY ulDataLen(%d)", ulDataLen);
                THROW(SCP_ERROR);
            }

            pstVelocityMsg = (SMB_RecvVelocityMsg_S*)pucData;

            OSMutexPend(g_pstSimProMutex, 0, &err);
            if (err != OS_ERR_NONE)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPend error err(%d)", err);
                return SCP_ERROR;
            }

            ulSPPulseDistance = g_ulSPPulseDistance;

            OSMutexPost(g_pstSimProMutex);
            if (err != OS_ERR_NONE)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPost error err(%d)", err);
                THROW(SCP_ERROR);
            }

            lRet = RADAR_SetVelocity(ntohl(pstVelocityMsg->lVelocityRadar), ulSPPulseDistance);//设置雷达速度
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SPD_SetRadar(%d) lVelocityRadar(%d)", lRet, ntohl(pstVelocityMsg->lVelocityRadar));
                THROW(SCP_ERROR);
            }
            else if (lRet == SCP_OK)
            {
                LOG(LOG_INFO, LOG_MOD_PRO, "SP_Process RADAR_SetVelocity OK!");
            }

            lRet = SPD_SetVelocity(ntohl(pstVelocityMsg->lVelocityA), ntohl(pstVelocityMsg->lVelocityB));//设置速度计速度
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SPD_SetVelocity(%d) lVelocityA(%d) lVelocityB(%d)", lRet, ntohl(pstVelocityMsg->lVelocityA), ntohl(pstVelocityMsg->lVelocityB));
                THROW(SCP_ERROR);
            }
            else if (lRet == SCP_OK)
            {
                LOG(LOG_INFO, LOG_MOD_PRO, "SP_Process SPD_SetVelocity OK!");
            }


            break;
        }
        //雷达故障注入报文1
        case SMB_INJECT_MSG_TYPE_RADAR1:
        {
            if (ulDataLen != sizeof(SMB_RecvInjectRadarMsg1_S))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_INJECT_MSG_TYPE_RADAR1 ulDataLen(%d)", ulDataLen);
                THROW(SCP_ERROR);
            }

            lRet = RADAR_SetInject1((SMB_RecvInjectRadarMsg1_S*)pucData);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error RADAR_SetInject1(%d)", lRet);
                THROW(SCP_ERROR);
            }
            break;
        }
        //雷达故障注入报文2
        case SMB_INJECT_MSG_TYPE_RADAR2:
        {
            if (ulDataLen != sizeof(SMB_RecvInjectRadarMsg2_S))
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error SMB_INJECT_MSG_TYPE_RADAR2 ulDataLen(%d)", ulDataLen);
                THROW(SCP_ERROR);
            }

            lRet = RADAR_SetInject2((SMB_RecvInjectRadarMsg2_S*)pucData);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error RADAR_SetInject2(%d)", lRet);
                THROW(SCP_ERROR);
            }
            break;
        }
        default:
        {
            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Process error usMsgType(0x%04x)", ntohs(pstMsgHead->usMsgType));
            THROW(SCP_ERROR);
        }
    }
TRY_END
    return ERROR_CODE;
}


//回送数据（包括速度，累计脉冲，电流，pwm，占空比，输入开关量）
STATIC int32_t SP_Send(SOCKET_ADDR stSPSockaddr)
{
    int32_t lRet = SCP_OK;
    SMB_SendAppMsg_S stMsg = {0};
    int32_t lCurrentPulseA = 0u;
    int32_t lCurrentPulseB = 0u;
    uint32_t ulBoardNum;
    uint32_t ulDatalen;
    STATIC uint8_t ucCycleNum = 0u;//周期号

TRY_BEGIN
    stMsg.stMsgHead.ucVersion = SMB_INTERFACE_VERSION;
    stMsg.stMsgHead.ucRes = ucCycleNum;
    ucCycleNum++;
    stMsg.stMsgHead.usMsgType = htons(SMB_SEND_MSG_TYPE_APP);

    //回送数据组包
    SPD_GetVelocity(&stMsg.lVelocityA, &stMsg.lVelocityB);
    stMsg.lVelocityRadar = RADAR_GetVelocity();
    SPD_GetCurrentPulse(&lCurrentPulseA, &lCurrentPulseB);
    stMsg.lCurrentPulse = lCurrentPulseA;
    lRet = CL_GetCLoop(&stMsg.ulCLoop);
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Send error CL_GetCLoop(%d)", lRet);
        THROW(SCP_ERROR);
    }

    lRet = PWM_GetPwm(&stMsg.ulPwmFreq, &stMsg.ulPwmDutyCycle);
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Send error PWM_GetPwm(%d)", lRet);
        THROW(SCP_ERROR);
    }

    ulBoardNum = SMB_IO_BOARD_NUM;
    lRet = IO_GetIoInfo(stMsg.astIoBoardInfo, &ulBoardNum);
    if ((lRet != SCP_OK) || (ulBoardNum > SMB_IO_BOARD_NUM))
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Send error IO_GetIoInfo(%d) ulBoardNum(%d)", lRet, ulBoardNum);
        THROW(SCP_ERROR);
    }
    ulDatalen = sizeof(stMsg) - sizeof(stMsg.astIoBoardInfo) + (sizeof(stMsg.astIoBoardInfo[0]) * ulBoardNum);

    //回送数据大小端转换
    stMsg.lVelocityA = htonl(stMsg.lVelocityA);
    stMsg.lVelocityB = htonl(stMsg.lVelocityB);
    stMsg.lVelocityRadar = htonl(stMsg.lVelocityRadar);
    stMsg.lCurrentPulse = htonl(stMsg.lCurrentPulse);
    stMsg.ulCLoop = htonl(stMsg.ulCLoop);
    stMsg.ulPwmFreq = htonl(stMsg.ulPwmFreq);
    stMsg.ulPwmDutyCycle = htonl(stMsg.ulPwmDutyCycle);
        
    lRet = fn_Sendto_TXQueue((uint8_t *)&stMsg, ulDatalen, &stSPSockaddr, 0, Udp_Recv_Buf_Mutex);
    if (lRet != SYS_YES)
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Send error UDP_Send(%d)", lRet);
        THROW(SCP_ERROR);
    }
TRY_END
    return ERROR_CODE;
}

//周期性回送数据
void SP_Loop(void)
{
    int32_t lRet = SCP_OK;
    uint32_t ulSPCycle;
    SOCKET_ADDR stSPSockaddr;
    uint8_t err = 0;

    while (1)
    {
        OSMutexPend(g_pstSimProMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPend error err(%d)", err);
            break;
        }

        ulSPCycle = g_ulSPCycle;
        stSPSockaddr = g_stSPSockaddr;

        OSMutexPost(g_pstSimProMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Init OSMutexPost error err(%d)", err);
            break;
        }

        if (ulSPCycle != 0u)
        {
            //周期性上报
            OSTimeDlyHMSM(0, 0, 0, ulSPCycle);
            lRet = SP_Send(stSPSockaddr);
            if (lRet != SCP_OK)
            {
                LOG(LOG_ERROR, LOG_MOD_PRO, "SP_Loop error SP_Send(%d)", lRet);
                OSTimeDlyHMSM(0, 0, 0, 1000);
            }
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 10);
        }
    }
}
