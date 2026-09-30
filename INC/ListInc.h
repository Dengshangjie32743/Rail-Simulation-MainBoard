/**
 * @brief 链表模块
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author HARDWARE group
 * @file   ListInc.h
 * @ingroup SLib
 *
 */


#ifndef _LISTINC_H_
#define _LISTINC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "TypeInc.h"

/**
 * @brief 双向链表结构体
 */
typedef struct S_ListHead
{
    struct S_ListHead *prev;    /**< 前一个节点指针 */
    struct S_ListHead *next;    /**< 后一个节点指针 */
} S_ListHead_S;

/**
 * @brief 获取链表起始地址
 */
#define S_LIST_ENTRY(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * @brief 双向链表初始化宏
 */
#define S_LIST_HEAD_INIT(name) {&(name), &(name)}

/**
 * @brief 双向链表定义加初始化宏
 */
#define S_LIST_HEAD(name) S_ListHead_S name = LIST_HEAD_INIT(name)

/**
 * @brief 非安全顺序遍历链表
 */
#define S_LIST_FOR_EACH(pos, head) \
for (pos = (head)->next; pos != (head);pos = pos->next)

/**
 * @brief 安全顺序遍历链表
 */
#define S_LIST_FOR_EACH_SAFE(pos, n, head) \
for (pos = (head)->next, n = pos->next; pos != (head);pos = n, n = pos->next)

/**
 * @brief 非安全反序遍历链表
 */
#define S_LIST_FOR_EACH_PREV(pos, head) \
for (pos = (head)->prev; pos != (head);pos = pos->prev)

/**
 * @brief 安全反序遍历链表
 */
#define S_LIST_FOR_EACH_PREV_SAFE(pos, n, head) \
for (pos = (head)->prev, n = pos->prev; pos != (head);pos = n, n = pos->prev)


/**
 * @brief 通过将前向指针和后向指针指向自己来创建一个链表表头
 * - [iSCP-SLib-SwCD-F-1811]
 *
 * @param[in] list 指向list_head结构体的指针
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
 */
void S_ListInitHead(S_ListHead_S* list);

/**
 * @brief 在两个已知节点中插入新节点
 * - [iSCP-SLib-SwCD-F-1812]
 *
 * @param[in] new  要插入结点的指针域
 * @param[in] prev 前一个节点的指针域
 * @param[in] next 后一个节点的指针域
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
 */
void S_ListAdd(S_ListHead_S* new, S_ListHead_S* prev, S_ListHead_S* next);

/**
 * @brief 在已知链表头部插入新节点
 * - [iSCP-SLib-SwCD-F-1813]
 *
 * @param[in] new 要插入结点的指针域
 * @param[in] head 要插入链表表头的指针域
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
*/
void S_ListAddHead(S_ListHead_S* new, S_ListHead_S* head);

/**
 * @brief 在已知链表尾部插入新节点
 * - [iSCP-SLib-SwCD-F-1814]
 *
 * @param[in] new 要插入结点的指针域
 * @param[in] head 要插入链表表头的指针域
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
*/
void S_ListAddTail(S_ListHead_S* new, S_ListHead_S* head);
 
/**
 * @brief 在链表中删除某一节点
 * - [iSCP-SLib-SwCD-F-1815]
 *
 * @param[in] entry 要删除的结点的指针域
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
 */
void S_ListDel(S_ListHead_S* entry);

/**
 * @brief 判断双向链表是否为空
 * - [iSCP-SLib-SwCD-F-1816]
 *
 * @param[in] head 链表表头的指针域
 *
 * @return bIsEmpty
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
 */
bool_t S_ListIsEmpty(const S_ListHead_S* head);

/**
 * @brief 从A链表删除后头插法插入B链表
 * - [iSCP-SLib-SwCD-F-1817]
 *
 * @param[in] node A链表要移除的节点的指针域
 * @param[in] head B链表表头的指针域
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
*/
void S_ListMoveHead(S_ListHead_S* node, S_ListHead_S* head);

/**
 * @brief 从A链表删除后尾插法插入B链表
 * - [iSCP-SLib-SwCD-F-1818]
 *
 * @param[in] node A链表要移除的节点的指针域
 * @param[in] head B链表表头的指针域
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
*/
void S_ListMoveTail(S_ListHead_S* node, S_ListHead_S* head);

/**
 * @brief 计算链表节点数
 * - [iSCP-SLib-SwCD-F-1819]
 *
 * @param[in] head 链表表头的指针域
 *
 * @return 返回链表上节点数目
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
*/
uint32_t S_ListCalcCount(const S_ListHead_S* head);

#ifdef __cplusplus
}
#endif

#endif /* _LISTINC_H_ */


