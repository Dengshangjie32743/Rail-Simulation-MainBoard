/**
 * @brief 环形队列
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-12-14   Dengshangjie：首次创建
 *
 */

#include "TypeInc.h"

#define QUEUE_SIZE 64

typedef struct Node
{
    uint32_t ulCanSrcId;
    uint32_t ulCanLength;
    uint8_t  aucData[8];
} S_Node_S;

typedef struct Queue
{
    S_Node_S data[QUEUE_SIZE];
    int32_t front;
    int32_t rear;
} SMB_Queue_S;

// 初始化队列
void S_QueueInit(SMB_Queue_S* q);
// 判断队列是否为空
bool_t S_QueueIsEmpty(SMB_Queue_S* q);
// 判断队列是否已满
bool_t S_QueueIsFull(SMB_Queue_S* q);
// 入队操作
bool_t S_EnQueue(SMB_Queue_S* q, S_Node_S node);
// 出队操作
int32_t S_DeQueue(SMB_Queue_S* q, S_Node_S* node);
