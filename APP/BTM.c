/**
 * @brief BTM （PC-->>SIM/SIM-->>PC）
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-10-01   ZhuChengYin
 *
 */

#include "TypeInc.h"
#include "Smb.h"
#include "BTM.h"
#include "fdcan.h"
#include "Log.h"
#include "string.h"
#include "SLib.h"
#include "ListInc.h"
#include "ucos_ii.h"
#include "simPro.h"
#include "QueueInc.h"
#include "udp_socket.h"

#define BTM_CAN_ATP1    0x01u
#define BTM_CAN_ATP2    0x00u
#define BTM_CAN_BTM1    0x02u
#define BTM_CAN_BTM2    0x03u

#define BTM_FRAME_NA        0x00u
#define BTM_FRAME_OK        0x06u
#define BTM_FRAME_ERR       0x15u

#define BTM_PRO_TYPE_LKJ  0x00u
#define BTM_PRO_TYPE_ATP  0x60u

#define BTM_FRAME_TYPE_IDLE  0xE1u
#define BTM_FRAME_TYPE_DATA  0xE2u

#define BTM_REQ_FRAME_LEN        30u
#define BTM_REQ_FRAME_NUM         4u
#define BTM_DATA_FRAME_LEN      138u
#define BTM_IDLE_FRAME_LEN       24u
#define BTM_FRAME_BUF           256u

#define BTM_LINK_BUILD_CNT      3u
#define BTM_LINK_BROKEN_CNT     5u

#define BTM_SIGNLE_DISTANCE     50u //BTM信号范围，单位cm
#define BTM_SIGNLE_OFFSET       -150 //BTM链路损耗，单位ms

#pragma pack(1)
typedef struct {
    uint8_t ucRequestSn;//请求帧ID码
    uint8_t ucFrameCorrectAndType;//帧正确/不正确/协议类型
    uint8_t ucOpenAmplifier;//开关功放命令
    uint8_t ucMsgType;//空闲/报文数据
    uint16_t usRspSn;//应答器序列号
    uint16_t usSpeed;//速度，单位cm/s
    uint32_t ulDistance;//累计走行距离，单位cm
    uint32_t ulTimeStamp;//主机时间戳，单位ms
    uint8_t ucProVersionX;//协议版本号x
    uint8_t ucProVersionY;//协议版本号y
    uint8_t ucProVersionZ;//协议版本号z
    uint8_t ucIdAndRecvMsg;//bit7-6:源标识，bit5-4:目的标识，bit3-0:01H:BTM只接收FSK报文，02H:BTM只接收PSK报文，03H:BTM只接收FSK和PSK报文
    uint8_t aucDate[6];//时间
    uint32_t ulCrc32_I;//CRC32-I，校验范围0~25字节
} BTM_RequestMsg_S;//查询报文

#define BTM_PASS_STATE_ON   0x03u
#define BTM_PASS_STATE_END  0xACu
#define BTM_DATA_MSG_LEN    104u

#define BTM_MSG_TYPE_DATA   0x64u
#define BTM_MSG_TYPE_IDLE   0x8Cu

typedef struct {
    uint8_t ucMsgType;//帧类型
    uint8_t ucMsgLen;//帧长
    uint8_t ucRequestSn;//请求帧ID码
    uint8_t ucReplySn;//应答帧ID码
    uint8_t ucPassState;//应答器通过状态
    uint8_t ucMsgPro;//BTM报文属性，bit7-6:11b:报文帧包含FSK报文和PSK报文；10b:报文帧中只包含FSK报文；01b:报文帧中只包含PSK报文；00b:预留。
                     //bit5:预留(LKJ)/以下定义(ATP)，1b:FSK报文有效,0b:FSK报文无效;bi4:预留(LKJ)/以下定义(ATP)1b:PSK报文有效,0b:PSK报文无效，bit3-0:预留。
    uint16_t usRecvBtmSn;//接收到的信标序列号
    uint8_t aucDate[BTM_DATA_MSG_LEN];//报文内容
    uint32_t ulBtmRecvTimeStamp;//接收信标的时间戳，单位ms，T(atp) - (T(tick) - T(receive线))
    uint32_t ulLobeBeginTime;//应答器波瓣起点时间，单位ms，T(atp) - (T(tick) - T(receive线))
    uint32_t ulLobeEndTime;//应答器波瓣终点时间，单位ms，(起点时间+T)
    uint32_t ulTimeStamp;//报文帧生成时刻，单位ms，T(atp) + (T(tick1) - T(receive_atp))
    uint8_t ucId;//bit7-6:预留,bi5-4:00b表示PSK及FSK都不支持,01b(FSK),10b(PSK),11b(支持FSK和PSK);bit3-2:源标识,bit1-0:目的标识
    uint8_t ucRes;//BTM厂家自定义状态
    uint32_t ulCrc32_C;//CRC32-C，校验范围0~129字节
    uint32_t ulCrc32_I;//CRC32-I，校验范围0~129字节
} BTM_DataMsg_S;//单报文

typedef struct {
    uint8_t ucMsgType;//帧类型
    uint8_t ucMsgLen;//帧长
    uint8_t ucRequestSn;//请求帧ID码
    uint8_t ucReplySn;//应答帧ID码
    uint8_t ucOpenAmplifierState;//BTM功放状态
    uint8_t ucWorkingState;//BTM工作状态
    uint32_t ulTimeStamp;//报文生成时间戳，单位ms
    uint32_t ulCrc32_C;//CRC32-C，校验范围5~9字节
    uint8_t ucProVersionX;//协议版本号x
    uint8_t ucProVersionY;//协议版本号y
    uint8_t ucProVersionZ;//协议版本号z
    uint8_t ucId;//BTM报文属性，bit7-6:预留；bit5-4:00b表示PSK及FSK都不支持，01b（FSK），10b（PSK），11b（支持FSK和PSK）；bit3-2:源标识；bit1-0:目的标识
    uint8_t aucRes[2];//BTM厂家自定义状态
    uint32_t ulCrc32_I;//CRC32-I，校验范围0~19字节
} BTM_IdleMsg_S;//空闲报文
#pragma pack()

#define BTM_OBJ_NUM   2u
#define BTM_FRAME_BUF_NUM   16u
typedef struct {
    uint32_t ulCanNum;
    uint32_t ulCanSrcId;
    uint32_t ulCanDestId;
    uint32_t ulCanId;

    bool_t bIsLinked;
    uint32_t ulOkCnt;
    uint32_t ulErrCnt;

    uint32_t ulCanFrameBufIndex;
    CAN_Frame_S astCanFrameBuf[BTM_FRAME_BUF_NUM];

    uint8_t ucRequestSn;//请求帧ID码
    uint8_t ucReplySn;//应答帧ID码
    uint16_t usSpeed;//速度，单位cm/s
    uint16_t usRecvBtmSn;//接收到的信标序列号
    uint32_t ulDistance;//累计走行距离，单位cm
    uint32_t ulTimeStamp;//主机时间戳，单位ms
    uint32_t ulRecvTime;

    S_ListHead_S astBtmDataList;
    S_ListHead_S astBtmDataFreeList;
} BTM_Obj_S;
BTM_Obj_S g_astBtmObj[BTM_OBJ_NUM];

#define BTM_DATA_LEN   104u
#define BTM_INFO_NUM   16u
typedef struct {
    S_ListHead_S stListNode;
    uint32_t uReceiveSingleMessageStamp;//接收btm单包文数据帧的时间，ms
    uint8_t aucData[BTM_DATA_LEN];
} BTM_Data_S;
BTM_Data_S g_aastBtmData[BTM_OBJ_NUM][BTM_INFO_NUM];
OS_EVENT *g_pstBtmMutex; // 信号量（互斥量）
SMB_Queue_S g_astBTM0Data;
SMB_Queue_S g_astBTM1Data;
/* begin add by deng 2024年1月30日 */
uint32_t g_InjectBTMMsg;//故障注入标志位
/* end add by deng 2024年1月30日 */


// 协议初始化
int32_t BTM_Init(void)
{
    uint32_t i = 0u;
/* begin modified by deng 2023年11月20日 */
    uint8_t ucPrio = BTM_MUTEX_PRIO;//需要根据实际任务优先级修改
/* end modified by deng 2023年11月20日 */
    uint8_t ucErr = OS_ERR_NONE;
    uint8_t err = 0;
TRY_BEGIN

    if ((sizeof(BTM_RequestMsg_S) != BTM_REQ_FRAME_LEN)
     || (sizeof(BTM_DataMsg_S) != BTM_DATA_FRAME_LEN)
     || (sizeof(BTM_IdleMsg_S) != BTM_IDLE_FRAME_LEN))
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Init error (%d)(%d)(%d)", sizeof(BTM_RequestMsg_S), sizeof(BTM_DataMsg_S), sizeof(BTM_IdleMsg_S));
        THROW(SCP_ERROR);
    }

    if (g_pstBtmMutex == NULL)
    {
          memset(g_astBtmObj, 0, sizeof(g_astBtmObj));

          /* begin add by deng 2023年11月30日 */
          //初始化收发队列
          HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
          S_QueueInit(&g_astBTM0Data); //BTM0数据队列初始化
          S_QueueInit(&g_astBTM1Data); //BTM1数据队列初始化
          S_MemZero(&g_astBTM0Data, 64 * (BTM_REQ_FRAME_LEN+2)); //BTM0数据缓存区初始化
          S_MemZero(&g_astBTM1Data, 64 * (BTM_REQ_FRAME_LEN+2)); //BTM1数据缓存区初始化
          HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断
          /* end add by deng 2023年11月30日 */
      /* begin add by deng 2024年1月30日 */
          g_InjectBTMMsg = 0u;
      /* end add by deng 2024年1月30日 */

          g_astBtmObj[0].ulCanNum = 0u;
          g_astBtmObj[0].bIsLinked = SCP_FALSE;
          /* begin add by deng 2023年11月30日 */
          g_astBtmObj[0].ulCanSrcId = 0X01; //ATP-1
          g_astBtmObj[0].ulCanDestId = 0X02; //BTM-1
          /* end add by deng 2023年11月30日 */
          S_ListInitHead(&g_astBtmObj[0].astBtmDataList);
          S_ListInitHead(&g_astBtmObj[0].astBtmDataFreeList);

          g_astBtmObj[1].ulCanNum = 1u;
          g_astBtmObj[1].bIsLinked = SCP_FALSE;
          S_ListInitHead(&g_astBtmObj[1].astBtmDataList);
          S_ListInitHead(&g_astBtmObj[1].astBtmDataFreeList);

          S_MemZero(&g_aastBtmData, sizeof(g_aastBtmData));
          for (i = 0u; i < BTM_INFO_NUM; ++i)
          {
              /*- 将所有链表节点挂到free链表上 */
              S_ListAddTail(&g_aastBtmData[0][i].stListNode, &g_astBtmObj[0].astBtmDataFreeList);
              S_ListAddTail(&g_aastBtmData[1][i].stListNode, &g_astBtmObj[1].astBtmDataFreeList);
          }

          g_pstBtmMutex = OSMutexCreate(BTM_MUTEX_PRIO, &ucErr);
          if (OS_ERR_NONE != ucErr)
          {
              LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Init OSMutexCreate error ucPrio(%d) ucErr(%d)", ucPrio, ucErr);
              THROW(SCP_ERROR);
          }
    }
    else
    {
          HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
          S_QueueInit(&g_astBTM0Data); //BTM0数据队列初始化
          S_QueueInit(&g_astBTM1Data); //BTM1数据队列初始化
          S_MemZero(&g_astBTM0Data, 64 * (BTM_REQ_FRAME_LEN+2)); //BTM0数据缓存区初始化
          S_MemZero(&g_astBTM1Data, 64 * (BTM_REQ_FRAME_LEN+2)); //BTM1数据缓存区初始化
          HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断

          OSMutexPend(g_pstBtmMutex, 0, &err);
          if (err != OS_ERR_NONE)
          {
              LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Init OSMutexPend error err(%d)", err);
              return SCP_ERROR;
          }
          memset(g_astBtmObj, 0, sizeof(g_astBtmObj));

          /* begin add by deng 2023年11月30日 */
          //初始化收发队列
          /* end add by deng 2023年11月30日 */
      /* begin add by deng 2024年1月30日 */
          g_InjectBTMMsg = 0u;
      /* end add by deng 2024年1月30日 */

          g_astBtmObj[0].ulCanNum = 0u;
          g_astBtmObj[0].bIsLinked = SCP_FALSE;
          /* begin add by deng 2023年11月30日 */
          g_astBtmObj[0].ulCanSrcId = 0X01; //ATP-1
          g_astBtmObj[0].ulCanDestId = 0X02; //BTM-1
          /* end add by deng 2023年11月30日 */
          S_ListInitHead(&g_astBtmObj[0].astBtmDataList);
          S_ListInitHead(&g_astBtmObj[0].astBtmDataFreeList);

          g_astBtmObj[1].ulCanNum = 1u;
          g_astBtmObj[1].bIsLinked = SCP_FALSE;
          S_ListInitHead(&g_astBtmObj[1].astBtmDataList);
          S_ListInitHead(&g_astBtmObj[1].astBtmDataFreeList);

          S_MemZero(&g_aastBtmData, sizeof(g_aastBtmData));
          for (i = 0u; i < BTM_INFO_NUM; ++i)
          {
              /*- 将所有链表节点挂到free链表上 */
              S_ListAddTail(&g_aastBtmData[0][i].stListNode, &g_astBtmObj[0].astBtmDataFreeList);
              S_ListAddTail(&g_aastBtmData[1][i].stListNode, &g_astBtmObj[1].astBtmDataFreeList);
          }
         err = OSMutexPost(g_pstBtmMutex);
         if (err != OS_ERR_NONE)
         {
             LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Init OSMutexPost error err(%d)", err);
             return SCP_ERROR;
         }
    }

TRY_END
    return ERROR_CODE;
}

/*
 * 找到完整的请求帧（请求帧由4帧can帧组成,ID从0x300至0x303）
 */
STATIC int32_t BTM_FindOneFrame(BTM_Obj_S* pstBtmObj, uint32_t ulCanId, uint8_t aucData[], uint32_t ulMaxDataLen)
{
    uint32_t i = 0u;
    uint32_t j = 0u;
    uint32_t ulDataLen = 0u;

    if (pstBtmObj->ulCanFrameBufIndex < BTM_REQ_FRAME_NUM)//判断请求帧的can帧数量是否完整
    {
        /* begin add by deng 2023年11月30日 */
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_FindOneFrame error FrameBufIndex(%d)", pstBtmObj->ulCanFrameBufIndex);
        /* end add by deng 2023年11月30日 */
        return ulDataLen;
    }

    i = pstBtmObj->ulCanFrameBufIndex - BTM_REQ_FRAME_NUM;
    /* begin modified by deng 2023年11月30日 */
//    for (j = 0; j < BTM_REQ_FRAME_NUM; j++)
    for (j = 0; j < BTM_REQ_FRAME_NUM; i++,j++)
    /* end modified by deng 2023年11月30日 */
    {
        if (pstBtmObj->astCanFrameBuf[i].ulCanId == (ulCanId + j))//判断请求帧的can帧顺序是否正确
        {
            if ((ulDataLen + pstBtmObj->astCanFrameBuf[i].ulLen) <= ulMaxDataLen)
            {
                memcpy(&aucData[ulDataLen], pstBtmObj->astCanFrameBuf[i].aucData, pstBtmObj->astCanFrameBuf[i].ulLen);
                ulDataLen += pstBtmObj->astCanFrameBuf[i].ulLen;
            }
            else
            {
                LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_FindOneFrame error ulDataLen(%d)", ulDataLen + pstBtmObj->astCanFrameBuf[i].ulLen);
                ulDataLen = 0u;
                break;
            }
        }
        else
        {
            /* begin add by deng 2023年11月30日 */
            LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_FindOneFrame error ulCanId(0x%x) != (0x%x)", pstBtmObj->astCanFrameBuf[i].ulCanId, ulCanId + j);
            /* end add by deng 2023年11月30日 */
            ulDataLen = 0u;
            break;
        }
    }

    return ulDataLen;
}

STATIC int32_t BTM_Recv(BTM_Obj_S* pstBtmObj, uint8_t aucData[], uint32_t* pulDataLen)
{
    int32_t lRet = SCP_OK;
    uint32_t ulCanId = (pstBtmObj->ulCanDestId << 7u) | (pstBtmObj->ulCanSrcId << 9u);
    CAN_Frame_S stCanFrame;
    uint32_t ulDataLen = 0u;

TRY_BEGIN
    while (1)
    {
        memset(&stCanFrame, 0, sizeof(stCanFrame));

        lRet = CAN_Recv(pstBtmObj->ulCanNum, &stCanFrame); //CAN接收
        if (lRet != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Recv CAN_Recv error lRet(%d)", lRet);
            THROW(SCP_ERROR);
        }
        if (stCanFrame.ulLen == 0u)//判断是否接收完
        {
            break;
        }

        memcpy(&pstBtmObj->astCanFrameBuf[pstBtmObj->ulCanFrameBufIndex], &stCanFrame, sizeof(stCanFrame)); //copy请求帧报文
        pstBtmObj->ulCanFrameBufIndex++;

        if (stCanFrame.ulCanId == (ulCanId + (BTM_REQ_FRAME_NUM - 1u))) //判断是否接收到请求帧里的最后一个can数据帧
        {
            ulDataLen = BTM_FindOneFrame(pstBtmObj, ulCanId, aucData, *pulDataLen);
            if (ulDataLen != 0u)
            {
                LOG_BIN(LOG_INFO, LOG_MOD_BTM, aucData, ulDataLen, "BTM_FindOneFrame");
            }
            pstBtmObj->ulCanFrameBufIndex = 0u;
        }

        if (pstBtmObj->ulCanFrameBufIndex >= BTM_FRAME_BUF_NUM)
        {
            pstBtmObj->ulCanFrameBufIndex = 0u;
        }
    }
    *pulDataLen = ulDataLen;

TRY_END
    return ERROR_CODE;
}

/*
 * 计算BTM链路连接状态
 */
STATIC void BTM_CalcLinkState(BTM_Obj_S* pstBtmObj, int32_t lError)
{
    if (lError != SCP_OK)
    {
        pstBtmObj->ulErrCnt++;
        if (pstBtmObj->ulErrCnt >= BTM_LINK_BROKEN_CNT)//连续5帧请求帧不完整，断链
        {
            pstBtmObj->ulErrCnt = BTM_LINK_BROKEN_CNT;  //防止一直加，然后回卷导致误判
            pstBtmObj->bIsLinked = SCP_FALSE;
            pstBtmObj->ulOkCnt = 0u;
        }
    }
    else
    {
        if (pstBtmObj->bIsLinked != SCP_TRUE)
        {
            pstBtmObj->ulOkCnt++;
            if (pstBtmObj->ulOkCnt >= BTM_LINK_BUILD_CNT)//连续3帧请求帧完整，建链
            {
                pstBtmObj->ulOkCnt = BTM_LINK_BUILD_CNT;    //防止一直加，然后回卷导致误判
                pstBtmObj->bIsLinked = SCP_TRUE;
                pstBtmObj->ulErrCnt = 0u;
            }
/* begin modified by deng 2024年8月15日 */
            pstBtmObj->ucReplySn = 0u;//建链时采用序号0
/* end modified by deng 2024年8月15日 */
        }
    }
}

STATIC int32_t BTM_ConstructDataMsg(BTM_Obj_S* pstBtmObj, BTM_Data_S* pstBtmData, uint8_t aucOutData[], uint32_t* pulOutDataLen)
{
    BTM_DataMsg_S *pstDataMsg;

TRY_BEGIN
    if (*pulOutDataLen < sizeof(*pstDataMsg))
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructDataMsg error *pulOutDataLen(%d) < sizeof(*pstDataMsg)(%d)", *pulOutDataLen, sizeof(*pstDataMsg));
        THROW(SCP_ERROR);
    }

    pstDataMsg = (BTM_DataMsg_S*)aucOutData;//将输出地址赋予datamsg

    pstDataMsg->ucMsgType = BTM_MSG_TYPE_DATA;
    pstDataMsg->ucMsgLen = BTM_DATA_FRAME_LEN;
    pstDataMsg->ucRequestSn = pstBtmObj->ucRequestSn;
/* begin modified by deng 2025年3月1日 */
//    pstDataMsg->ucReplySn = pstBtmObj->ucReplySn;
//    if (pstBtmObj->ucRequestSn == 0)
//    {
//        pstDataMsg->ucReplySn = 0;
//        pstBtmObj->ucReplySn = 0;
//    }
//    else
//    {
//        pstDataMsg->ucReplySn = pstBtmObj->ucReplySn;
//    }
    pstDataMsg->ucReplySn = pstBtmObj->ucRequestSn;//答帧ID码与请求帧ID保持一致
/* end modified by deng 2025年3月1日 */
    pstBtmObj->ucReplySn++;
    pstDataMsg->ucPassState = BTM_PASS_STATE_END;
    pstDataMsg->ucMsgPro = 0xa0u;
/* begin add by deng 2025年3月1日 */
    if (pstBtmObj->usRecvBtmSn == 0)//应答器序号1~65535循环
    {
        pstBtmObj->usRecvBtmSn = 1;
    }
/* end add by deng 2025年3月1日 */
    pstDataMsg->usRecvBtmSn = htons(pstBtmObj->usRecvBtmSn);
    pstBtmObj->usRecvBtmSn++;
    memcpy(pstDataMsg->aucDate, pstBtmData->aucData, sizeof(pstDataMsg->aucDate));
    /* begin modified by deng 2025年3月1日 */
//    pstDataMsg->ulBtmRecvTimeStamp = htonl(pstBtmObj->ulTimeStamp);//接收信标的时间戳，单位ms，T(atp) - (T(tick) - T(receive线))
//    pstDataMsg->ulLobeBeginTime = htonl(pstBtmObj->ulTimeStamp);//应答器波瓣起点时间，单位ms，T(atp) - (T(tick) - T(receive线))
//    pstDataMsg->ulLobeEndTime = htonl(pstBtmObj->ulTimeStamp);//应答器波瓣终点时间，单位ms，(起点时间+T)
//    pstDataMsg->ulTimeStamp = htonl((OSTimeGet() - pstBtmObj->ulRecvTime) + pstBtmObj->ulTimeStamp);//报文帧生成时刻，单位ms，T(atp) + (T(tick1) - T(receive_atp))
    pstDataMsg->ulBtmRecvTimeStamp = htonl(pstBtmObj->ulTimeStamp - (OSTimeGet() - pstBtmData->uReceiveSingleMessageStamp));//接收信标的时间戳，单位ms，T(atp) - (T(tick) - T(receive线))
    pstDataMsg->ulLobeBeginTime = htonl(pstBtmObj->ulTimeStamp - (OSTimeGet() - pstBtmData->uReceiveSingleMessageStamp) + BTM_SIGNLE_OFFSET);//应答器波瓣起点时间，单位ms，T(atp) - (T(tick) - T(receive线)) + 线路损耗时间
    pstDataMsg->ulLobeEndTime = htonl(pstBtmObj->ulTimeStamp - (OSTimeGet() - pstBtmData->uReceiveSingleMessageStamp) + ((BTM_SIGNLE_DISTANCE * 1000u) / pstBtmObj->usSpeed) + BTM_SIGNLE_OFFSET);//应答器波瓣终点时间，单位ms，(起点时间+T())
    pstDataMsg->ulTimeStamp = htonl((OSTimeGet() - pstBtmObj->ulRecvTime) + pstBtmObj->ulTimeStamp);//报文帧生成时刻，单位ms，T(atp) + (T(tick1) - T(receive_atp))
    /* end modified by deng 2025年3月1日 */
/* begin add by deng 2025年3月1日 用于调试*/
//    LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructDataMsg error pstBtmObj->usSpeed(%u) pstBtmObj->ulTimeStamp(%u)", pstBtmObj->usSpeed , pstBtmObj->ulTimeStamp);
//    LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructDataMsg error BtmRecvTimeStamp(%u) LobeBeginTime(%u) LobeEndTime(%u) TimeStamp(%u)", ntohl(pstDataMsg->ulBtmRecvTimeStamp), ntohl(pstDataMsg->ulLobeBeginTime), ntohl(pstDataMsg->ulLobeEndTime), ntohl(pstDataMsg->ulTimeStamp));
/* end add by deng 2025年3月1日 */
    pstDataMsg->ucId = 0x10u | (pstBtmObj->ulCanSrcId << 2u) | pstBtmObj->ulCanDestId;
    pstDataMsg->ulCrc32_C = htonl(S_Crc32C(aucOutData, BTM_DATA_FRAME_LEN - sizeof(uint32_t) - sizeof(uint32_t)));
    pstDataMsg->ulCrc32_I = htonl(S_Crc32I(aucOutData, BTM_DATA_FRAME_LEN - sizeof(uint32_t) - sizeof(uint32_t)));

    pstBtmObj->ulCanId = (pstBtmObj->ulCanDestId << 9u) | (pstBtmObj->ulCanSrcId << 7u) | 0x20u;

    *pulOutDataLen = sizeof(*pstDataMsg);

TRY_END
    return ERROR_CODE;
}

STATIC int32_t BTM_ConstructIdleMsg(BTM_Obj_S* pstBtmObj, uint8_t aucOutData[], uint32_t* pulOutDataLen)
{
    BTM_IdleMsg_S *pstIdleMsg;

TRY_BEGIN
    if (*pulOutDataLen < sizeof(*pstIdleMsg))
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructIdleMsg error *pulOutDataLen(%d) < sizeof(*pstIdleMsg)(%d)", *pulOutDataLen, sizeof(*pstIdleMsg));
        THROW(SCP_ERROR);
    }

    pstIdleMsg = (BTM_IdleMsg_S*)aucOutData;

    pstIdleMsg->ucMsgType = BTM_MSG_TYPE_IDLE;
    pstIdleMsg->ucMsgLen = BTM_IDLE_FRAME_LEN;
    pstIdleMsg->ucRequestSn = pstBtmObj->ucRequestSn;
/* begin modified by deng 2025年3月1日 */
//    pstIdleMsg->ucReplySn = pstBtmObj->ucReplySn;
//    if (pstBtmObj->ucRequestSn == 0)
//    {
//        pstIdleMsg->ucReplySn = 0;
//        pstBtmObj->ucReplySn = 0;
//    }
//    else
//    {
//        pstIdleMsg->ucReplySn = pstBtmObj->ucReplySn;
//    }
    pstIdleMsg->ucReplySn = pstBtmObj->ucRequestSn;//答帧ID码与请求帧ID保持一致
/* end modified by deng 2025年3月1日 */
    pstBtmObj->ucReplySn++;
    pstIdleMsg->ucOpenAmplifierState = 0x57u;
    pstIdleMsg->ucWorkingState = 0x00u;
    pstIdleMsg->ulTimeStamp = htonl((OSTimeGet() - pstBtmObj->ulRecvTime + pstBtmObj->ulTimeStamp));
    pstIdleMsg->ulCrc32_C = htonl(S_Crc32C(&aucOutData[5], 5u));
    pstIdleMsg->ucProVersionX = 1u;
    pstIdleMsg->ucProVersionX = 0u;
    pstIdleMsg->ucProVersionZ = 0u;
    pstIdleMsg->ucId = 0x10u | (pstBtmObj->ulCanSrcId << 2u) | pstBtmObj->ulCanDestId;
    pstIdleMsg->ulCrc32_I = htonl(S_Crc32I(aucOutData, BTM_IDLE_FRAME_LEN - sizeof(uint32_t)));

    pstBtmObj->ulCanId = (pstBtmObj->ulCanDestId << 9u) | (pstBtmObj->ulCanSrcId << 7u) | 0x40u;
//start modified by zmy 2024年8月15日
//	if(pstIdleMsg->ucRequestSn == 0)//正在建链
//	{
//	    pstBtmObj->ucReplySn = 0x00;
//	}
//    if(pstIdleMsg->ucRequestSn != 0)//建链成功 2024/9/3 zmy
//	{
//		if(pstBtmObj->ucReplySn == 0)//建链后取值范围1~255
//		{
//		    pstBtmObj->ucReplySn = 0x01;
//		}
//	}
//end modified by zmy 2024年8月15日
    *pulOutDataLen = sizeof(*pstIdleMsg);
TRY_END
    return ERROR_CODE;
}

STATIC int32_t BTM_ConstructMsg(BTM_Obj_S* pstBtmObj, uint8_t aucOutData[], uint32_t* pulOutDataLen)
{
    int32_t lRet = SCP_OK;
    uint8_t ucErr = OS_ERR_NONE;
    BTM_Data_S* pstBtmData;
    BTM_Data_S stBtmData;

TRY_BEGIN
    memset(aucOutData, 0, *pulOutDataLen);

    OSMutexPend(g_pstBtmMutex, 0, &ucErr);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructMsg error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }

    if (SCP_TRUE != S_ListIsEmpty(&pstBtmObj->astBtmDataList))
    {
        //构造单报文帧
        pstBtmData = S_LIST_ENTRY(pstBtmObj->astBtmDataList.next, BTM_Data_S, stListNode);
        memcpy(&stBtmData, pstBtmData, sizeof(stBtmData));
        S_ListMoveTail(pstBtmObj->astBtmDataList.next, &pstBtmObj->astBtmDataFreeList);

        lRet = BTM_ConstructDataMsg(pstBtmObj, &stBtmData, aucOutData, pulOutDataLen);
        if (lRet != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructMsg BTM_ConstructDataMsg error lRet(%d)", lRet);
            THROW(SCP_ERROR);
        }
    }
    else
    {
        //构造空闲帧
        lRet = BTM_ConstructIdleMsg(pstBtmObj, aucOutData, pulOutDataLen);
        if (lRet != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructMsg BTM_ConstructIdleMsg error lRet(%d)", lRet);
            THROW(SCP_ERROR);
        }
    }

    ucErr |= OSMutexPost(g_pstBtmMutex);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_ConstructMsg error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }
TRY_END
    return ERROR_CODE;
}


STATIC int32_t BTM_Process(BTM_Obj_S* pstBtmObj, uint8_t aucData[], uint32_t ulDataLen, uint8_t aucOutData[], uint32_t* pulOutDataLen)
{
    int32_t lRet = SCP_OK;
    uint32_t ulCrc32_I = 0u;
    BTM_RequestMsg_S* pstRequestMsg;

TRY_BEGIN
    if (ulDataLen == 0u)//判断请求帧数据长度（防御性编程）
    {
        /* begin deleted by deng 2023年11月30日 */
//        BTM_CalcLinkState(pstBtmObj, SCP_ERROR);
        /* end deleted by deng 2023年11月30日 */
        *pulOutDataLen = 0u;
        THROW(SCP_OK);
    }
    if (ulDataLen != BTM_REQ_FRAME_LEN)//判断请求帧数据长度
    {
        LOG_BIN(LOG_ERROR, LOG_MOD_BTM, aucData, ulDataLen, "BTM_Process ulDataLen(%d) error", ulDataLen);
        BTM_CalcLinkState(pstBtmObj, SCP_ERROR);
        *pulOutDataLen = 0u;
        THROW(SCP_OK);
    }

    pstRequestMsg = (BTM_RequestMsg_S*)aucData;
    ulCrc32_I = S_Crc32I(aucData, ulDataLen - sizeof(ulCrc32_I));
    if (htonl(ulCrc32_I) != pstRequestMsg->ulCrc32_I)//检验请求帧的crc
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Process error ulCrc32_I calc(0x%08x) != ulCrc32_I msg(0x%08x)", htonl(ulCrc32_I), pstRequestMsg->ulCrc32_I);
        BTM_CalcLinkState(pstBtmObj, SCP_ERROR);
        *pulOutDataLen = 0u;
        THROW(SCP_OK);
    }

    if ((pstRequestMsg->ucFrameCorrectAndType & 0x60u) != 0x60u)//检验请求帧的正确性
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Process error Type(0x%02x)", pstRequestMsg->ucFrameCorrectAndType);
        BTM_CalcLinkState(pstBtmObj, SCP_ERROR);
        *pulOutDataLen = 0u;
        THROW(SCP_OK);
    }

    BTM_CalcLinkState(pstBtmObj, SCP_OK);//计算链接状态
    if (pstBtmObj->bIsLinked != SCP_TRUE)
    {
        *pulOutDataLen = 0u;
        THROW(SCP_OK);
    }

    /* begin deleted by deng 2023年11月30日 */
//    pstBtmObj->ulCanSrcId = (pstRequestMsg->ucIdAndRecvMsg >> 6u) & 0x03;
//    pstBtmObj->ulCanDestId = (pstRequestMsg->ucIdAndRecvMsg >> 4u) & 0x03;
    /* end deleted by deng 2023年11月30日 */
    pstBtmObj->ucRequestSn = pstRequestMsg->ucRequestSn;
    pstBtmObj->usSpeed = ntohs(pstRequestMsg->usSpeed);
    pstBtmObj->ulDistance = ntohl(pstRequestMsg->ulDistance);
    pstBtmObj->ulTimeStamp = ntohl(pstRequestMsg->ulTimeStamp);
    pstBtmObj->ulRecvTime = OSTimeGet();

    lRet = BTM_ConstructMsg(pstBtmObj, aucOutData, pulOutDataLen);//构造单报文帧或空闲帧
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Process BTM_ConstructMsg error lRet(%d)", lRet);
        *pulOutDataLen = 0u;
        THROW(SCP_ERROR);
    }

TRY_END
    return ERROR_CODE;
}

/* begin add by deng 2024年3月31日 */
STATIC int32_t BTM_Fi(BTM_Obj_S* pstBtmObj, uint8_t aucOutData[], uint32_t* pulOutDataLen)
{
    int32_t lRet = SCP_OK;
    uint8_t aucFiTail[26]={0};//故障报文尾
    uint8_t aucFiCRC[4]={0XFF, 0XFF, 0XFF, 0XFF};//故障crc

    //单报文帧故障注入
    if (aucOutData[0] == 0x64)
    {
        if (g_InjectBTMMsg == 0u)    //无故障注入
        {
            //无操作
        }
        else if(g_InjectBTMMsg == 1u)//故障注入—报文过长
        {
            memcpy(aucFiTail, (aucOutData + 112u), 26u);
            memcpy((aucOutData + 120u), aucFiTail, 26u);//数据段多一帧
            *pulOutDataLen = sizeof(BTM_DataMsg_S) + 8u;
        }
        else if(g_InjectBTMMsg == 2u)//故障注入—报文过短
        {
            memcpy((aucOutData + 104u), (aucOutData + 112u), 26u);//数据段少一帧
            *pulOutDataLen = sizeof(BTM_DataMsg_S) - 8u;
        }
        else if(g_InjectBTMMsg == 3u)//故障注入—无报文头
        {
            memcpy(aucOutData , (aucOutData + 8u), 130u);
            *pulOutDataLen = sizeof(BTM_DataMsg_S) - 8u; //无报文头(报文头为8个字节)
        }
        else if(g_InjectBTMMsg == 4u)//故障注入—无报文尾
        {
            *pulOutDataLen = sizeof(BTM_DataMsg_S) - 26u; //无报文尾(报文尾为26个字节)
        }
        else if(g_InjectBTMMsg == 5u)//故障注入—报文id错误
        {
            pstBtmObj->ulCanId = 0xee; //更改报文起始id
        }
        else if(g_InjectBTMMsg == 6u)//故障注入—报文crc错误
        {
            memcpy((aucOutData + 134u), aucFiCRC, 4u);            
        }

        g_InjectBTMMsg = 0u;//发送一包故障数据后，清除故障
    }
    //空闲帧故障注入
    else if(aucOutData[0] == 0x8C)
    {
       if (g_InjectBTMMsg == 0u)    //无故障注入
        {
            //无操作
        }
        else if(g_InjectBTMMsg == 1u)//故障注入—报文过长
        {
            memcpy((aucOutData + 24u), (aucOutData + 16u), 8u);//数据段多一帧
            S_Memset((aucOutData + 16u), 0, 8u);//置零
            *pulOutDataLen = sizeof(BTM_IdleMsg_S) + 8u;
        }
        else if(g_InjectBTMMsg == 2u)//故障注入—报文过短
        {
            *pulOutDataLen = sizeof(BTM_IdleMsg_S) - 8u;//报文少一帧
        }
        else if(g_InjectBTMMsg == 3u)//故障注入—预留
        {
            //无操作
        }
        else if(g_InjectBTMMsg == 4u)//故障注入—预留
        {
            //无操作
        }
        else if(g_InjectBTMMsg == 5u)//故障注入—报文id错误
        {
            pstBtmObj->ulCanId = 0xfd; //更改报文起始id
        }
        else if(g_InjectBTMMsg == 6u)//故障注入—报文crc错误
        {
            memcpy((aucOutData + 20u), aucFiCRC, 4u);
        }

        g_InjectBTMMsg = 0u;//发送一包故障数据后，清除故障
    }

    return lRet;
}
/* end add by deng 2024年3月31日 */

STATIC int32_t BTM_SendMsg(BTM_Obj_S* pstBtmObj, uint8_t aucData[], uint32_t ulDataLen)
{
    int32_t lRet = SCP_OK;
    uint32_t i = 0u;
    uint32_t ulIndex = 0u;
    CAN_Frame_S stCanFrame;

TRY_BEGIN
/* begin add by deng 2023年11月30日 */
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
/* end add by deng 2023年11月30日 */
    while (ulDataLen != 0u)
    {
        memset(&stCanFrame, 0, sizeof(stCanFrame));
        stCanFrame.ulCanId = pstBtmObj->ulCanId + i;
        stCanFrame.ulLen = sizeof(stCanFrame.aucData);
        if (ulDataLen >= sizeof(stCanFrame.aucData))//判断（空闲帧/单报文帧）是否发送完成
        {

            memcpy(stCanFrame.aucData, &aucData[ulIndex], sizeof(stCanFrame.aucData));
            ulDataLen = ulDataLen - sizeof(stCanFrame.aucData);
        }
        else
        {
            memcpy(stCanFrame.aucData, &aucData[ulIndex], ulDataLen);
            ulDataLen = 0u;
        }

        lRet = CAN_Send(pstBtmObj->ulCanNum, &stCanFrame);
        if (lRet != SCP_OK)//判断是否正常发送can
        {
            LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_SendMsg CAN_Send error lRet(%d)", lRet);
            THROW(SCP_ERROR);
        }

        i++;
        ulIndex = ulIndex + sizeof(stCanFrame.aucData);
    }

/* begin add by deng 2023年11月30日 */
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断
/* end add by deng 2023年11月30日 */

TRY_END
    return ERROR_CODE;
}

int32_t BTM_Loop(uint32_t ulBtmObjIndex)
{
    int32_t lRet = SCP_OK;
    BTM_Obj_S *pstBtmObj;
    uint8_t aucData[BTM_REQ_FRAME_LEN] = { 0 };
    uint32_t ulDataLen = 0u;
    uint8_t aucOutData[BTM_FRAME_BUF] = { 0 };
    uint32_t ulOutDataLen = sizeof(aucOutData);
    uint8_t ucErr;

TRY_BEGIN
    if (ulBtmObjIndex >= BTM_OBJ_NUM)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Loop error ulBtmObjIndex(%d)", ulBtmObjIndex);
        THROW(SCP_ERROR);
    }

    OSMutexPend(g_pstBtmMutex, 0, &ucErr);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "OSMutexPend error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }

    pstBtmObj = &g_astBtmObj[ulBtmObjIndex];

    ucErr |= OSMutexPost(g_pstBtmMutex);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "OSMutexPost error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }

    ulDataLen = sizeof(aucData);
    lRet = BTM_Recv(pstBtmObj, aucData, &ulDataLen); //接收BTM报文(请求帧)
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Loop BTM_Recv error lRet(%d)", lRet);
        THROW(SCP_ERROR);
    }

    lRet = BTM_Process(pstBtmObj, aucData, ulDataLen, aucOutData, &ulOutDataLen); //处理btm报文
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Loop BTM_Process error lRet(%d)", lRet);
        THROW(SCP_ERROR);
    }

/* begin add by deng 2024年1月30日 */
    lRet = BTM_Fi(pstBtmObj, aucOutData, &ulOutDataLen);//构造故障
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Loop BTM_SendMsg error lRet(%d)", lRet);
        THROW(SCP_ERROR);
    }
/* end add by deng 2024年1月30日 */

    lRet = BTM_SendMsg(pstBtmObj, aucOutData, ulOutDataLen); //发送BTM报文(空闲帧/单报文帧)
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "BTM_Loop BTM_SendMsg error lRet(%d)", lRet);
        THROW(SCP_ERROR);
    }

TRY_END
    return ERROR_CODE;
}

//BTM接收单报文(同时记录接收时间)
int32_t BTM_SetBtmData(SMB_BtmMsg_S* pstBtmMsg)
{
    uint32_t i = 0u;
    uint8_t ucErr = OS_ERR_NONE;
    BTM_Obj_S* pstBtmObj;
    BTM_Data_S* pstBtmData;
    S_ListHead_S *pstNode;

TRY_BEGIN
    OSMutexPend(g_pstBtmMutex, 0, &ucErr);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "OSMutexPend error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }

    for (i = 0u; i < BTM_OBJ_NUM; ++i)
    {
        pstBtmObj = &g_astBtmObj[i];

        if (SCP_TRUE != S_ListIsEmpty(&pstBtmObj->astBtmDataFreeList))
        {
            pstNode = pstBtmObj->astBtmDataFreeList.next;
        }
        else
        {
            pstNode = pstBtmObj->astBtmDataList.prev;
        }
        S_ListDel(pstNode);
        pstBtmData = S_LIST_ENTRY(pstNode, BTM_Data_S, stListNode);//获取链表起始地址

        memcpy(pstBtmData->aucData, &pstBtmMsg->aucBtmData, sizeof(pstBtmData->aucData));//复制BTM数据内容
        /* begin add by deng 2025年3月1日 */
        pstBtmData->uReceiveSingleMessageStamp = OSTimeGet();//记录接收时间
        /* end add by deng 2025年3月1日 */
        S_ListAddTail(pstNode, &pstBtmObj->astBtmDataList);//插入节点到链表
    }
    ucErr |= OSMutexPost(g_pstBtmMutex);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "OSMutexPost error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }
TRY_END
    return ERROR_CODE;
}
/* begin add by deng 2024年1月30日 */
int32_t BTM_RecvInjectBTMMsg1(SMB_RecvInjectBTMMsg1_S* pstBtmMsg)
{
    int32_t lRet = SCP_OK;
    uint32_t uInjectBTMMsg1;//记录BTM故障值

    uInjectBTMMsg1 = ntohl(pstBtmMsg->uInjectBTMMsg1);//大小端转换

    if (uInjectBTMMsg1 == 0u)    //无故障
    {
        g_InjectBTMMsg = 0u;
    }
    else if(uInjectBTMMsg1 == 1u)//故障注入报文过长
    {
        g_InjectBTMMsg = 1u;
    }
    else if(uInjectBTMMsg1 == 2u)//故障注入报文过短
    {
        g_InjectBTMMsg = 2u;
    }
    else if(uInjectBTMMsg1 == 3u)//故障注入无报文头
    {
        g_InjectBTMMsg = 3u;
    }
    else if(uInjectBTMMsg1 == 4u)//故障注入无报文尾
    {
        g_InjectBTMMsg = 4u;
    }
    else if(uInjectBTMMsg1 == 5u)//故障注入报文id错误
    {
        g_InjectBTMMsg = 5u;
    }
    else if(uInjectBTMMsg1 == 6u)//故障注入—报文id错误
    {
        g_InjectBTMMsg = 6u;
    }
    else
    {
        lRet = SCP_ERROR;//故障注入失败
    }

    return lRet;
}
/* end add by deng 2024年1月30日 */

int32_t BTM_CleanBtmData(void)
{
    uint32_t i = 0u;
    uint8_t ucErr = OS_ERR_NONE;
    BTM_Obj_S* pstBtmObj;
    S_ListHead_S *pstNode;

TRY_BEGIN
    OSMutexPend(g_pstBtmMutex, 0, &ucErr);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "OSMutexPend error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }

    for (i = 0u; i < BTM_OBJ_NUM; ++i)
    {
        pstBtmObj = &g_astBtmObj[i];

        while (SCP_TRUE != S_ListIsEmpty(&pstBtmObj->astBtmDataList))
        {
            pstNode = pstBtmObj->astBtmDataList.next;
            S_ListMoveTail(pstNode, &pstBtmObj->astBtmDataFreeList);
        }
    }

    ucErr |= OSMutexPost(g_pstBtmMutex);
    if (OS_ERR_NONE != ucErr)
    {
        LOG(LOG_ERROR, LOG_MOD_BTM, "OSMutexPost error ucErr(%d)", ucErr);
        THROW(SCP_ERROR);
    }
TRY_END
    return ERROR_CODE;
}

