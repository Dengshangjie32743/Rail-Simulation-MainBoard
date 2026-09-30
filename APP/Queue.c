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

#include "QueueInc.h"

// 初始化队列
void S_QueueInit(SMB_Queue_S* q)
{
    q->front = -1;
    q->rear = -1;
}

// 判断队列是否为空
bool_t S_QueueIsEmpty(SMB_Queue_S* q)
{
    return (q->front == q->rear);
}  

// 判断队列是否已满
bool_t S_QueueIsFull(SMB_Queue_S* q)
{
    return ((q->rear + 1) % QUEUE_SIZE == q->front);
}  

// 清空队列
//static void S_ClearQueue(SMB_Queue_S* q) {
//    q->front = -1;
//    q->rear = -1;
//    // 可以选择清空 items 数组，但不一定必要
//    // for (int i = 0; i < MAX_SIZE; ++i) {
//    //     q->items[i] = 0; // 可选
//    // }
//}

// 入队操作
bool_t S_EnQueue(SMB_Queue_S* q, S_Node_S node)
{
    if (S_QueueIsFull(q))
    {
        // 队列已满
        // 如果队列已满，将新数据放在队尾（覆盖最旧的数据）
        q->rear = (q->rear + 1) % QUEUE_SIZE;
    }
    q->data[q->rear] = node;
    q->rear = (q->rear + 1) % QUEUE_SIZE;

    return SCP_OK;
}
  
// 出队操作
int32_t S_DeQueue(SMB_Queue_S* q, S_Node_S* node)
{
    if (S_QueueIsEmpty(q))
    {
         // 队列为空
         return SCP_ERROR;
    }
    *node = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;

    return SCP_OK;
}
