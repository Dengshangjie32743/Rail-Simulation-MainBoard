/*************************************************************************************
*file: eth_data_mgt.h
*brief: 本文件为网络数据的收发的头文件。
*author: Rzl
*version:0.1
*create data：2021.07.26
*modify record：
*
**************************************************************************************/

#ifndef __UDP_SOCKET_H
#define __UDP_SOCKET_H

#include "sys.h"
#include "includes.h"
#include "Use_Task.h"

#define UDP_DEMO_PORT 8089
#define PS_ERR                      (-1)
#define UDP_D_PORT                                8089
#define UDP_BUFFSIZE                 1400          /*接收发送数据缓冲区大小*/
#define RCV_QUEUE_MAX                     40        /*接收缓冲区的个数*/
#define SED_QUEUE_MAX                     10        /*发送缓冲区的个数*/

#define MAX_SOCK_NUM                                 1          /*最大的socket缓存数量*/


/***********************************************************************************
*UCOS互斥信号量的定义
************************************************************************************/
extern OS_EVENT *Udp_Recv_Buf_Mutex ;
extern OS_EVENT *Udp_Send_Buf_Mutex ; 


typedef struct {
  uint8_t us_SinFamliy;/*协议族*/
  uint16_t us_SinPort;/*端口号*/
  uint32_t ui_SinAddr;/*ip address*/ 
}SocketTypeDef;
#define SOCKET_ADDR  SocketTypeDef
/**************************************************************************************************************************************/
/*@Brife:接收缓冲区结构定义                                                                                                           */
/**************************************************************************************************************************************/
typedef struct {
  SocketTypeDef st_SocketAddr;
  uint16_t us_Len;
  uint8_t auc_Buff[UDP_BUFFSIZE];
}SocketRecvBuf;
/**************************************************************************************************************************************/
/*@Brife:接收队列结构定义                                                                                                             */
/**************************************************************************************************************************************/
typedef struct {
  uint8_t uc_Head;
  uint8_t uc_Rear;
  uint8_t uc_Used;
  SocketRecvBuf ast_Buff[RCV_QUEUE_MAX];
}SocketRecvQueue;

/**************************************************************************************************************************************/
/*@Brife:发送缓冲区结构定义                                                                                                           */
/**************************************************************************************************************************************/
typedef struct {
  SocketTypeDef st_SocketAddr;
  uint16_t us_Len;
  uint8_t auc_Buff[UDP_BUFFSIZE];
}SocketSendBuf;
/**************************************************************************************************************************************/
/*@Brife:发送队列结构定义                                                                                                             */
/**************************************************************************************************************************************/
typedef struct {
  uint8_t uc_Head;
  uint8_t uc_Rear;
  uint8_t uc_Used;
  SocketSendBuf ast_Buff[SED_QUEUE_MAX];
}SocketSendQueue;

/**************************************************************************************************************************************/
/*@Brife:网络接口定义                                                                                                            */
/**************************************************************************************************************************************/
typedef struct {
  uint8_t uc_Init;
  uint8_t uc_FirstSnd;
  SocketRecvQueue st_RecvQueue;
  SocketSendQueue st_SendQueue;
}SocketInfo;

uint8_t fn_Eth_Data_Task_Init(void);
uint16_t fn_RecvFrom_RXQueue(uint8_t *databuff,uint16_t Maxlen,SOCKET_ADDR *sock_addr,uint8_t sock_type,OS_EVENT *buf_mutex);//zmy
uint8_t  fn_Sendto_TXQueue(uint8_t *databuff,uint16_t SendLen,SOCKET_ADDR *sock_addr,uint8_t sock_type,OS_EVENT *buf_mutex);

#endif

