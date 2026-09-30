#ifndef __USE_TASK_H
#define __USE_TASK_H


#include "sys.h"
#include "includes.h"

/***********************************************************************************
*UCOS网络任务优先级
************************************************************************************/
/* begin modified by deng 2023年11月20日 */
#define START_TASK_PRIO         10
/* end modified by deng 2023年11月20日 */
#define QUEUE_REV_PRIO          30
#define UDP_RECV_PRIO           31
#define UDP_SEND_PRIO           32
#define SP_PRIO                 33
#define RADAR_PRIO              34
#define COLLECT_PRIO            35
#define BTM1_PRIO               36
//#define BTM2_PRIO               37
#define VELOCITY_PRIO           38
//#define IO_PRIO                 39
/* begin modified by deng 2023年11月20日 */
#define LED_PRIO                50
/* end modified by deng 2023年11月20日 */
#define CPUUSAGE_PRIO           51
#define LOG_PRIO                52


/***********************************************************************************
*UCOS网络任务互斥信号量继承优先级
************************************************************************************/
#define UDP_RECV_MUTEX_PRIO     13
#define UDP_SEND_MUTEX_PRIO     14
#define SP_MUTEX_PRIO           15
#define IO_MUTEX_PRIO           16
#define RADAR_MUTEX_PRIO        17
#define SPEED_MUTEX_PRIO        18
#define CL_MUTEX_PRIO           19
/* begin modified by deng 2023年11月20日 */
#define PWM_MUTEX_PRIO          20
#define BTM_MUTEX_PRIO          21
#define LOG_MUTEX_PRIO          22
#define SPI_MUTEX_PRIO          23
/* end modified by deng 2023年11月20日 */

/***********************************************************************************
*UCOS网络任务堆栈大小
************************************************************************************/
#define UDP_RECV_STK_SIZE       2*1024
#define UDP_SEND_STK_SIZE       2*1024
#define QUEUE_REV_STK_SIZE      2*1024
#define RADAR_STK_SIZE          1*1024
#define COLLECT_STK_SIZE        1*1024
#define IO_STK_SIZE             2*1024
#define BTM_STK_SIZE            2*1024
#define VELOCITY_STK_SIZE       2*1024
#define SP_STK_SIZE             2*1024
/* begin modified by deng 2023年11月20日 */
#define LED_STK_SIZE            128
#define START_STK_SIZE          128
/* end modified by deng 2023年11月20日 */
#define CPUUSAGE_STK_SIZE       128
#define LOG_STK_SIZE            4*1024

typedef struct {
  uint8_t  version;//协议版本
    uint8_t  yuliu;//预留
    uint16_t type;//报文类型
    uint8_t  databuf[248];//数据
}Rev_QueueBuf;
extern Rev_QueueBuf *recvoutdata;


typedef struct {
  uint8_t  frame_type;//帧类型
    uint8_t  frame_len;//帧长
    uint8_t  reqid;//请求帧ID
    uint8_t  resid;//应答帧ID
    uint8_t  btmdatabuf[238];//数据
    uint32_t crc;//CRC
}Rev_BtmdataBuf;
extern Rev_BtmdataBuf *revbtmdata;


//uint8_t Queue_Init();

void RECV_Task(void);
/* begin modified by deng 2023.10.27 */
void Start_task(void *pdata);
void Radar_Task(void *arg);
void Collect_Task(void *arg);
void IO_Task(void *arg);
void BTM1_Task(void *arg);
void BTM2_Task(void *arg);
void VELOCITY_Task(void *arg);
void SP_Task(void *arg);
void Led_Task(void *pdata);
/* end modified by deng 2023.10.27 */
void CpuUsage_Task(void* pdata);
void Log_Task(void *arg);
#endif

