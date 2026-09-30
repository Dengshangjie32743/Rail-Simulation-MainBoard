#include "TypeInc.h"
#include <stdio.h> // 用于 printf 函数
#include <stdarg.h> // 用于 va_list 和 va_start、va_end 函数
#include "Log.h"
/* begin modified by deng 2023.10.27 */
#include "Slib.h"
#include "Use_Task.h"
#include <string.h>
/* end modified by deng 2023.10.27 */
/* begin add by deng 2024年12月1日 */
#include "udp_socket.h"
/* end add by deng 2024年12月1日 */

OS_EVENT *g_pstlogMutex = NULL; //日志打印互斥锁句柄
uint8_t g_aucLogLevel[LOG_MAX_MODULE_ID];   /**< 每个模块日志等级全局变量 */

#define PRINT_QUEUE_SIZE 10
#define PRINT_MSG_SIZE 200
OS_EVENT *g_pstPrintQueue;
int8_t g_aacPrintBuffer[PRINT_QUEUE_SIZE][PRINT_MSG_SIZE];
/* Modify End  by dsj 2024年8月15日*/
/*
 * 日志初始化
 */
int32_t LOG_Init(void)
{
/* begin add by deng 2024年3月31日 */
    uint8_t ucErr = SCP_OK;
    int8_t cErr = SCP_OK;

    // 创建消息队列
    g_pstPrintQueue = OSQCreate((void *)&g_aacPrintBuffer, PRINT_QUEUE_SIZE);
    if (g_pstPrintQueue == NULL)
    {
        printf("Failed to create print queue\n");
        cErr =  SCP_ERROR;
    }

    //创建互斥锁
    g_pstlogMutex = OSMutexCreate(LOG_MUTEX_PRIO, &ucErr);
    if (ucErr != OS_ERR_NONE)
    {
        printf("LOG_Init OSMutexCreate error err(%d)", ucErr);
        cErr =  SCP_ERROR;
    }

/* begin modified by deng 2024年1月30日 */
//    S_Memset(g_aucLogLevel, LOG_INFO, sizeof(g_aucLogLevel));   //设置默认等级为INFO
//    S_Memset(g_aucLogLevel, LOG_ERROR, sizeof(g_aucLogLevel));   //设置默认等级为ERROR
    S_Memset(g_aucLogLevel, LOG_WARNING, sizeof(g_aucLogLevel));   //设置默认等级为WARNING
/* end modified by deng 2024年1月30日 */

    return cErr;
}

/*
 * 打印
 */
void LOG(uint8_t ucLevel, uint8_t ucModuleId, const char *format, ...)
{
/* Modify Begin by dsj 2024年8月15日*/
    int32_t lLogLength = 0u;  //日志长度
    char_t aLog[PRINT_MSG_SIZE];
/* Modify End  by dsj 2024年8月15日*/
    va_list args;
TRY_BEGIN
    if (ucLevel > g_aucLogLevel[ucModuleId])
    {
        THROW(SCP_ERROR);
    }

/* deleted Begin by dsj 2024年8月15日*/
//    printf("\r\n[L-%d][M-0x%02x][T-%d]", ucLevel, ucModuleId, S_GetLocalTime());
//    va_start(args, format);
//    vprintf(format, args);
//    va_end(args);
/* deleted End  by dsj 2024年8月15日*/
    va_start(args, format);
    lLogLength = snprintf(aLog , PRINT_MSG_SIZE, "\r\n[L-%d][M-0x%02x][T-%d]", ucLevel, ucModuleId, S_GetLocalTime());//[]内容放入字符串
    vsnprintf((aLog + lLogLength), sizeof(aLog) - lLogLength, format, args);//日志放入字符串，与[]内容拼接
    va_end(args);

    OSQPost(g_pstPrintQueue, (void *)aLog);//打印的内容放入消息队列
    OSTimeDlyHMSM(0,0,0,10);//延时可以帮助控制消息的产生速率，避免过多的消息积压

TRY_END
    (void)ERROR_CODE;
    return;
}

/*
 * 打印加数据内容
 */
void LOG_BIN(uint8_t ucLevel, uint8_t ucModuleId, const uint8_t* pLogBin, uint32_t ulBinLen, const char *format, ...)
{
    uint32_t i;
    va_list args;
    char_t acLog[PRINT_MSG_SIZE];
TRY_BEGIN
    if (ucLevel > g_aucLogLevel[ucModuleId])
    {
        THROW(SCP_ERROR);
    }
/* begin deleted by deng 2024年12月1日 */
//    printf("\r\n[L-%d][M-0x%02x][T-%d]", ucLevel, ucModuleId, S_GetLocalTime());
//
//    va_start(args, format);
//    vprintf(format, args);
//    va_end(args);
//    printf(" [%d]:", ulBinLen);
//    for (i = 0; i < ulBinLen; ++i)
//    {
//        printf("%02x ", pLogBin[i]);
//    }
/* end deleted by deng 2024年12月1日 */
    snprintf(acLog, sizeof(acLog), "\r\n[L-%d][M-0x%02x][T-%d]", ucLevel, ucModuleId, S_GetLocalTime());
    va_start(args, format);
    vsnprintf(acLog + strlen(acLog), sizeof(acLog) - strlen(acLog), format, args); // 将格式化内容追加到 alog
    va_end(args);

    snprintf(acLog + strlen(acLog), sizeof(acLog) - strlen(acLog), " [%d]:", ulBinLen);
    for (i = 0; i < ulBinLen; ++i)
    {
        snprintf(acLog + strlen(acLog), sizeof(acLog) - strlen(acLog), "%02x ", pLogBin[i]); // 将二进制日志内容追加到 alog
    }

    OSQPost(g_pstPrintQueue, (void *)acLog);//打印的内容放入消息队列
    OSTimeDlyHMSM(0,0,0,10);//延时可以帮助控制消息的产生速率，避免过多的消息积压

TRY_END
    (void)ERROR_CODE;
    return;
}


/* Modify Begin by dsj 2024年8月15日*/
/*
 * 日志打印任务
 */
void LOG_LOOP(SOCKET_ADDR stSPSockaddr)
{
    int8_t *acLogMsg;
    uint8_t cuErr = 0u;
/* begin add by deng 2024年12月1日 */
    char_t UartLogMsgCopy[PRINT_MSG_SIZE];
    char_t NetLogMsgCopy[PRINT_MSG_SIZE];
/* end add by deng 2024年12月1日 */

    while(1)
    {
        // 从队列中获取消息
        acLogMsg = (int8_t *)OSQPend(g_pstPrintQueue, 0, &cuErr);
        strcpy((char *)UartLogMsgCopy, (const char *)acLogMsg);
        strcpy((char *)NetLogMsgCopy, (const char *)acLogMsg);

        if (OS_ERR_NONE == cuErr)//判断消息是否正常出对列
        {
        /* 串口打印 */
            printf("%s", UartLogMsgCopy);
/* begin add by deng 2024年12月1日 */
        /* 网络日志 */
            fn_Sendto_TXQueue((uint8_t *)NetLogMsgCopy, strlen((const char *)NetLogMsgCopy), &stSPSockaddr, 0, Udp_Recv_Buf_Mutex);
/* end add by deng 2024年12月1日 */
        }

        OSTimeDlyHMSM(0, 0, 0, 1);  //延时1ms，为后面任务运行腾时间
    }
}
/* Modify End  by dsj 2024年8月15日*/

