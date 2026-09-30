#include "udp_socket.h"
#include "lwip/sockets.h"
#include "lwip_comm.h" 
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
#include "malloc.h"
#include "btm.h"
/* begin modified by deng 2023.10.24 */
#include "Use_Task.h"
/* end modified by deng 2023.10.24 */
#include "Log.h"
/***********************************************************************************
*UCOS任务堆栈的定义
************************************************************************************/
OS_STK UDP_RECV_TASK_STK[UDP_RECV_STK_SIZE]        __EXRAM;
OS_STK UDP_SEND_TASK_STK[UDP_SEND_STK_SIZE]        __EXRAM;
OS_STK QUEUE_REV_TASK_STK[QUEUE_REV_STK_SIZE]        __EXRAM;
/* begin modified by deng 2023.10.27 */
OS_STK RADAR_TASK_STK[RADAR_STK_SIZE]                __EXRAM;
OS_STK COLLECT_TASK_STK[COLLECT_STK_SIZE]                  __EXRAM;
OS_STK IO_TASK_STK[IO_STK_SIZE]                    __EXRAM;
OS_STK BTM1_Task_STK[BTM_STK_SIZE]             __EXRAM;
OS_STK BTM2_Task_STK[BTM_STK_SIZE]             __EXRAM;
OS_STK VELOCITY_TASK_STK[VELOCITY_STK_SIZE]        __EXRAM;
OS_STK SP_TASK_STK[SP_STK_SIZE]    __EXRAM;
/* end modified by deng 2023.10.27 */
/* begin modified by deng 2023年11月20日 */
OS_STK LED_TASK_STK[LED_STK_SIZE];
/* end modified by deng 2023年11月20日 */
OS_STK CPUUSAGE_TASK_STK[CPUUSAGE_STK_SIZE];
OS_STK LOG_TASK_STK[LOG_STK_SIZE]          __EXRAM;
/***********************************************************************************
*UCOS互斥信号量的定义
************************************************************************************/
OS_EVENT *Udp_Recv_Buf_Mutex = NULL;
OS_EVENT *Udp_Send_Buf_Mutex = NULL;
/***********************************************************************************
*红蓝网的socket描述符
************************************************************************************/
int32_t UDP_Sock = PS_ERR;

/***********************************************************************************
*网络数据处理队列
************************************************************************************/
SocketInfo Socket_Queue[MAX_SOCK_NUM]        __EXRAM;
/* begin modified by deng 2023年11月20日 */
////TCP客户端任务
//#define UDP_PRIO        6
////任务堆栈大小
//#define UDP_STK_SIZE    300
////任务堆栈
//OS_STK UDP_TASK_STK[UDP_STK_SIZE];
/* end modified by deng 2023年11月20日 */
int32_t Demo_Sock = PS_ERR;

//static void udp_thread(void *arg);
static int8_t UDP_Sock_Make(int32_t *sock_num,__lwip_dev * lwip_addr,uint16_t port);
static void UDP_Recv_Task(void *arg);
static void QUEUE_Rev_Task(void *arg);
static uint8_t Udp_Data_Mutex_Create(void);
static void UDP_Data_In_Queue(int32_t sock_num,SocketRecvQueue *recv_queue,OS_EVENT *buf_mutex);
static int8_t TX_Queue_To_Socket(int32_t sock_num,SocketSendQueue *send_queue,OS_EVENT *buf_mutex);

/***********************************************************************************
*队列数据接收任务
************************************************************************************/
static void QUEUE_Rev_Task(void *arg)
{
    LWIP_UNUSED_ARG(arg);
    while(1)
    {
      RECV_Task();
      OSTimeDlyHMSM(0, 0, 0, 5);  //延时5ms //todo zzzcccyyy
    }
}
/***********************************************************************************
*网口数据接收任务
************************************************************************************/
static void UDP_Recv_Task(void *arg)
{

    uint8_t err = 0;
    LWIP_UNUSED_ARG(arg);

    while (1)
    {
        if (UDP_Sock_Make(&UDP_Sock, &lwipdev, UDP_DEMO_PORT) == PS_ERR)
        {
            if (UDP_Sock >= 0)
            {
                closesocket(UDP_Sock);
            }
        }
        else
        {
            OSMutexPend(Udp_Recv_Buf_Mutex, 0, &err);
            if (OS_ERR_NONE != err)
            {
                LOG(LOG_ERROR, LOG_MOD_UDP, "UDP_Recv_Task error err(%d)", err);
                return;
            }

            Socket_Queue[0].uc_Init = SYS_YES;

            err = OSMutexPost(Udp_Recv_Buf_Mutex);
            if (OS_ERR_NONE != err)
            {
                LOG(LOG_ERROR, LOG_MOD_UDP, "UDP_Recv_Task error err(%d)", err);
                return;
            }

            break;
        }

        OSTimeDlyHMSM(0, 0, 0, 5);  //延时5ms
    }

    while (1)
    {
        UDP_Data_In_Queue(UDP_Sock, &Socket_Queue[0].st_RecvQueue, Udp_Recv_Buf_Mutex);
        OSTimeDlyHMSM(0, 0, 0, 5);  //延时5ms
    }
}

/***********************************************************************************
*创建网络发送任务
************************************************************************************/
static void UDP_Send_Task(void *arg)
{
    while(1)
    {
        TX_Queue_To_Socket(UDP_Sock,&Socket_Queue[0].st_SendQueue,Udp_Send_Buf_Mutex);
/* begin add by deng 2023年11月30日 */
        OSTimeDlyHMSM(0,0,0,5);//延时，防止任务一直占用cpu
/* end add by deng 2023年11月30日 */
    }
    
}


/****************************************************************************************************
*描述：fn_Eth_Data_Task_Init 接口函数，用于创建网络数据发送，接收的任务，及队列防护互斥信号量。
*参数：无
*返回值：0 创建成功；其他值创建失败。
******************************************************************************************************/
uint8_t fn_Eth_Data_Task_Init(void)
{
    uint8_t ret = 0;

    memset(Socket_Queue, 0, sizeof(Socket_Queue));
    ret = Udp_Data_Mutex_Create();//创建网络收发互斥锁
    ret += OSTaskCreate(UDP_Recv_Task, (void*) 0, (OS_STK*) &UDP_RECV_TASK_STK[UDP_RECV_STK_SIZE - 1], UDP_RECV_PRIO); /**< 创建红网数据接收任务 */
    ret += OSTaskCreate(QUEUE_Rev_Task, (void*) 0, (OS_STK*) &QUEUE_REV_TASK_STK[QUEUE_REV_STK_SIZE - 1], QUEUE_REV_PRIO);    //网络命令接收
    ret += OSTaskCreate(UDP_Send_Task, (void*) 0, (OS_STK*) &UDP_SEND_TASK_STK[UDP_SEND_STK_SIZE - 1], UDP_SEND_PRIO); /**< 创建网络数据发送任务 */

    ret += OSTaskCreate(SP_Task, (void*) 0, (OS_STK*) &SP_TASK_STK[SP_STK_SIZE - 1], SP_PRIO);     //回送任务
    ret += OSTaskCreate(Radar_Task, (void*) 0, (OS_STK*) &RADAR_TASK_STK[RADAR_STK_SIZE - 1], RADAR_PRIO);    //雷达任务
//    ret += OSTaskCreate(Collect_Task, (void*) 0, (OS_STK*) &COLLECT_TASK_STK[COLLECT_STK_SIZE - 1], COLLECT_PRIO);    //PWM与电流采集任务
    ret += OSTaskCreate(BTM1_Task, (void*) 0, (OS_STK*) &BTM1_Task_STK[BTM_STK_SIZE - 1], BTM1_PRIO);    //BTM1任务
//    ret += OSTaskCreate(BTM2_Task, (void*) 0, (OS_STK*) &BTM2_Task_STK[BTM_STK_SIZE - 1], BTM2_PRIO);    //BTM2任务（暂不用）
//    ret += OSTaskCreate(IO_Task, (void*) 0, (OS_STK*) &IO_TASK_STK[IO_STK_SIZE - 1], IO_PRIO);    //空任务
//    ret += OSTaskCreate(VELOCITY_Task, (void*) 0, (OS_STK*) &VELOCITY_TASK_STK[VELOCITY_STK_SIZE - 1], VELOCITY_PRIO);   //空任务
/* begin modified by deng 2023年11月20日 */
    ret += OSTaskCreate(Led_Task, (void*) 0, (OS_STK*) &LED_TASK_STK[LED_STK_SIZE-1], LED_PRIO);   //LED任务
/* end modified by deng 2023年11月20日 */
//    ret += OSTaskCreate(CpuUsage_Task, (void*) 0, (OS_STK*) &CPUUSAGE_TASK_STK[CPUUSAGE_STK_SIZE-1], CPUUSAGE_PRIO); //CPU占用率任务(todo:存在宕机问题)
//    ret += OSTaskCreate(Log_Task, (void*) 0, (OS_STK*) &LOG_TASK_STK[LOG_STK_SIZE-1], LOG_PRIO);    //打印任务

    return ret;
}

/***********************************************************************************
*描述：Eth_Data_Mutex_Create 网络队列互斥信号量的创建
*参数：无
*返回值：temp 为0则全部创建成功，其他值则代表至少一个互斥信号量创建失败。
************************************************************************************/

static uint8_t Udp_Data_Mutex_Create(void)
{
    uint8_t err = 0;
    uint8_t temp = 0;

    Udp_Recv_Buf_Mutex = OSMutexCreate(UDP_RECV_MUTEX_PRIO, &err);
    temp += err;
    Udp_Send_Buf_Mutex = OSMutexCreate(UDP_SEND_MUTEX_PRIO, &err);
    temp += err;
    return temp;
}

/***********************************************************************************
*描述：UDP_Data_In_Queue 网络数据包入队列操作
*参数：sock_num 接收的socket编号，recv_queue 接收的队列指针
*返回值：无
*注意：1、该函数在scoket创建成功后调用；2，调用该函数会引起线程阻塞
************************************************************************************/

static void UDP_Data_In_Queue(int32_t sock_num, SocketRecvQueue *recv_queue, OS_EVENT *buf_mutex)
{
    struct sockaddr_in sock_remote_ip;
    uint8_t temp_ptr = 0, err = 0;
    uint8_t temp_buf[UDP_BUFFSIZE];
    uint16_t temp_len = 0;
    socklen_t addr_len = 0;
    /* begin add by deng 2025年3月1日 用于调试*/
    static uint32_t i = 0u;
    /* end add by deng 2025年3月1日 */

    addr_len = sizeof(struct sockaddr);
    memset(&sock_remote_ip, 0, sizeof(sock_remote_ip));
    memset(temp_buf, 0, UDP_BUFFSIZE);

    temp_ptr = recv_queue->uc_Rear;
    temp_len = recvfrom(sock_num, temp_buf, UDP_BUFFSIZE, 0, (struct sockaddr *) &sock_remote_ip, &addr_len);     //从socket接口获取数据，存入接收队列
    /* begin add by deng 2025年3月1日 用于调试*/
//    if (temp_len >= 104)
//    {
//        LOG(LOG_ERROR, LOG_MOD_PRO, "UDP_Data_In_Queue recvfrom BTM (%d)", ++i);
//    }
    /* end add by deng 2025年3月1日 */

    if (temp_len > 0)
    {
        recv_queue->ast_Buff[temp_ptr].us_Len = temp_len;
        recv_queue->ast_Buff[temp_ptr].st_SocketAddr.us_SinFamliy = sock_remote_ip.sin_family;
        recv_queue->ast_Buff[temp_ptr].st_SocketAddr.us_SinPort = ntohs(sock_remote_ip.sin_port);
        recv_queue->ast_Buff[temp_ptr].st_SocketAddr.ui_SinAddr = ntohl(sock_remote_ip.sin_addr.s_addr);
        memcpy(recv_queue->ast_Buff[temp_ptr].auc_Buff, temp_buf, temp_len);
        OSMutexPend(buf_mutex, 0, &err);
        recv_queue->uc_Rear = (temp_ptr + 1) % RCV_QUEUE_MAX;
        if (recv_queue->uc_Used + 1 > RCV_QUEUE_MAX)
        {
            recv_queue->uc_Head = (recv_queue->uc_Head + 1) % RCV_QUEUE_MAX;
        }
        else
        {
            recv_queue->uc_Used += 1;
        }
        OSMutexPost(buf_mutex);
    }
    else
    {
        LOG(LOG_ERROR, LOG_MOD_PRO, "UDP_Data_In_Queue recvfrom len(%d)", temp_len);
    }
}

/*****************************************************************************************************
*描述：TX_Queue_To_Socket 网络数据包通过协议UDP协议栈进行发送
*参数：sock_num 接收的socket编号，*send_queue发送队列指针，*buf_mutex 队列防护的互斥信号量指针
*返回值：ret PS_ERR 代表发送失败或无可发送的数据。
*注意：改函数LWIP协议栈，更换其他协议栈及物理网卡配置需修改该函数
******************************************************************************************************/
static int8_t TX_Queue_To_Socket(int32_t sock_num,SocketSendQueue *send_queue,OS_EVENT *buf_mutex)
{
    struct sockaddr_in sock_remote_ip;
    int8_t ret = PS_ERR;
    uint8_t *send_buff = NULL;
    uint8_t temp = 0, err = 0;
    uint16_t temp_len = 0;

//    OSMutexPend(buf_mutex, 0, &err);
//    if (OS_ERR_NONE != err)
//    {
//        LOG(LOG_ERROR, LOG_MOD_UDP, "TX_Queue_To_Socket error err(%d)", err);
//        return PS_ERR;
//    }

    while (send_queue->uc_Used > 0)
    {
        temp = send_queue->uc_Head;
        send_buff = send_queue->ast_Buff[temp].auc_Buff;
        temp_len = send_queue->ast_Buff[temp].us_Len;
        sock_remote_ip.sin_family = send_queue->ast_Buff[temp].st_SocketAddr.us_SinFamliy;
        sock_remote_ip.sin_port = htons(send_queue->ast_Buff[temp].st_SocketAddr.us_SinPort);
        sock_remote_ip.sin_addr.s_addr = htonl(send_queue->ast_Buff[temp].st_SocketAddr.ui_SinAddr);
        ret = sendto(sock_num, send_buff, temp_len, 0, (struct sockaddr*) &sock_remote_ip, sizeof(sock_remote_ip));
        if (ret != PS_ERR)
        {
            OSMutexPend(buf_mutex, 0, &err);
            send_queue->uc_Head = (temp + 1) % SED_QUEUE_MAX;
            send_queue->uc_Used -= 1;
            OSMutexPost(buf_mutex);
        }
    }
//    err = OSMutexPost(buf_mutex);
//    if (OS_ERR_NONE != err)
//    {
//        LOG(LOG_ERROR, LOG_MOD_UDP, "TX_Queue_To_Socket error err(%d)", err);
//        return PS_ERR;
//    }
    return ret;
}

/***********************************************************************************
*描述：UDP_Sock_Make 创建UDPsocket 并绑定本地的IP地址
*参数：sock_num 接收的socket编号，lwip_addr 网卡的IP地址信息 ，port 端口号
*返回值：无
*注意：改函数LWIP协议栈，更换其他协议栈及物理网卡配置需修改该函数
************************************************************************************/

static int8_t UDP_Sock_Make(int32_t *sock_num,__lwip_dev * lwip_addr,uint16_t port)
{
    int8_t err = 0;
    struct sockaddr_in sock_local_ip;
    struct ip_addr local_ip;

    memset(&local_ip, 0, sizeof(local_ip));
    memset(&sock_local_ip, 0, sizeof(sock_local_ip));

    *sock_num = socket(PF_INET, SOCK_DGRAM, 0);
    if (*sock_num != PS_ERR)
    {
        IP4_ADDR(&local_ip, lwip_addr->ip[0], lwip_addr->ip[1], lwip_addr->ip[2], lwip_addr->ip[3]);
        sock_local_ip.sin_len = sizeof(sock_local_ip);
        sock_local_ip.sin_family = AF_INET;
        sock_local_ip.sin_port = htons(port);
        sock_local_ip.sin_addr.s_addr = local_ip.addr;

        if (bind(*sock_num, (struct sockaddr *) &sock_local_ip, sizeof(sock_local_ip)) == PS_ERR)
        {
            err = PS_ERR;
        }
        else
            err = 0;

    }
    else
    {
        err = PS_ERR;
    }
    return err;
}

 
 /***************************************************************************************************
*描述：fn_RecvFrom_RXQueue 网络组件的接口函数，用与其他组件从网络接收队列中获取数据
*参数：*p_Data_Buff接收数据的缓存，需是真实的内存空间；Maxlen 能接受的最大数据量；
*      *sock_addr 接收数据包的地址信息；sock_type 接收队列序号,取值范围 0=<sock_type<MAX_SOCK_NUM
       *buf_mutex 队列防护的互斥信号量指针
*返回值：Ret_Len 返回数据的长度；0为无数据或错误。
*****************************************************************************************************/
uint16_t fn_RecvFrom_RXQueue(uint8_t *databuff, uint16_t Maxlen, SOCKET_ADDR *sock_addr, uint8_t sock_type, OS_EVENT *buf_mutex)     //zmy
{
    uint8_t temp = 0, err = 0;
    uint8_t *dataptr = NULL;
    uint16_t Ret_Len = 0;
    if (sock_type == 0)
    {
//        OSMutexPend(buf_mutex, 0, &err);
//        if (OS_ERR_NONE != err)
//        {
//            LOG(LOG_ERROR, LOG_MOD_UDP, "fn_RecvFrom_RXQueue error err(%d)", err);
//            return 0;
//        }

        if (Socket_Queue[sock_type].uc_Init == SYS_YES)
        {
            OSMutexPend(buf_mutex, 0, &err);
            if (Socket_Queue[sock_type].st_RecvQueue.uc_Used > 0)
            {
                temp = Socket_Queue[sock_type].st_RecvQueue.uc_Head;

                Ret_Len = Socket_Queue[sock_type].st_RecvQueue.ast_Buff[temp].us_Len;
                if (Ret_Len < Maxlen)
                {
                    *sock_addr = Socket_Queue[sock_type].st_RecvQueue.ast_Buff[temp].st_SocketAddr;
                    dataptr = Socket_Queue[sock_type].st_RecvQueue.ast_Buff[temp].auc_Buff;
                    memcpy(databuff, dataptr, Ret_Len);
                }
                else
                {
                    Ret_Len = 0;     //zmy
                }
                Socket_Queue[sock_type].st_RecvQueue.uc_Head = (temp + 1) % RCV_QUEUE_MAX;
                Socket_Queue[sock_type].st_RecvQueue.uc_Used -= 1;
            }
        }
        err = OSMutexPost(buf_mutex);
//        if (OS_ERR_NONE != err)
//        {
//            LOG(LOG_ERROR, LOG_MOD_UDP, "fn_RecvFrom_RXQueue error err(%d)", err);
//            return 0;
//        }
    }
    return Ret_Len;
}
 
 /***************************************************************************************************
*描述：fn_Sendto_TXQueue 网络组件的接口函数，用与其他组件向网络缓存队列中发送数据
*参数：*p_Data_Buff发送数据的源地址，需是真实的内存空间；SendLen 发送的数据长度；
*      *sock_addr 发送数据包的地址信息；sock_type 接收队列序号,取值范围 0=<sock_type<MAX_SOCK_NUM
       *buf_mutex 队列防护的互斥信号量指针
*返回值：ret 返回发送的状态 SYS_YES 发送成功；SYS_ERR 发送失败
*****************************************************************************************************/
uint8_t  fn_Sendto_TXQueue(uint8_t *databuff,uint16_t SendLen,SOCKET_ADDR *sock_addr,uint8_t sock_type,OS_EVENT *buf_mutex)
{
    uint8_t temp = 0, err = 0;
    uint8_t ret = SYS_YES;
//    sock_addr->us_SinFamliy = AF_INET;
//    sock_addr->ui_SinAddr = 0xc0a80143;
//    sock_addr->us_SinPort = 8089;
    if(SendLen <= UDP_BUFFSIZE)
    {
        if(sock_type == 0)
        {
//            OSMutexPend(buf_mutex,0,&err);
//            if (OS_ERR_NONE != err)
//            {
//                LOG(LOG_ERROR, LOG_MOD_UDP, "fn_Sendto_TXQueue error err(%d)", err);
//                return 0;
//            }

            if(Socket_Queue[sock_type].uc_Init == SYS_YES)
            {
                    
                    temp=Socket_Queue[sock_type].st_SendQueue.uc_Rear;
                    Socket_Queue[sock_type].st_SendQueue.ast_Buff[temp].st_SocketAddr = *sock_addr;
                    Socket_Queue[sock_type].st_SendQueue.ast_Buff[temp].us_Len = SendLen;
                    memcpy(Socket_Queue[sock_type].st_SendQueue.ast_Buff[temp].auc_Buff,databuff,SendLen);                    
                    Socket_Queue[sock_type].st_SendQueue.uc_Rear = (temp+1)%SED_QUEUE_MAX;
                    OSMutexPend(buf_mutex,0,&err);
                    if(Socket_Queue[sock_type].st_SendQueue.uc_Used +1 > SED_QUEUE_MAX )
                    {
                        Socket_Queue[sock_type].st_SendQueue.uc_Head = (Socket_Queue[sock_type].st_SendQueue.uc_Head +1)%SED_QUEUE_MAX;
                    }
                    else
                    {
                        Socket_Queue[sock_type].st_SendQueue.uc_Used +=1;
                    }
                    OSMutexPost(buf_mutex);
            }
            else
            {
                  ret = SYS_ERR;
            }            
//            err = OSMutexPost(buf_mutex);
//            if (OS_ERR_NONE != err)
//            {
//                LOG(LOG_ERROR, LOG_MOD_UDP, "fn_Sendto_TXQueue error err(%d)", err);
//                return 0;
//            }
      }
        else
        {
                ret = SYS_ERR;
        }
    }
    else
    {
        ret = SYS_ERR;
    }
  return ret;
    
}
 
 



 
 
