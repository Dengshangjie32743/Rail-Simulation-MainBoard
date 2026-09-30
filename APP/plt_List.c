/**
 * @brief 链表模块
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author HARDWARE group
 * @file   plt_List.c
 * @ingroup SLib
 * @version V1.0.0
 * @date 2021-11-01
 *
 */

#include "ListInc.h"


/**
 * @brief 通过将前向指针和后向指针指向自己来创建一个链表表头
 * - [iSCP-SLib-SwCD-F-1811]
 *
 * @param[in] list 指向list_head结构体的指针
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
 */
void S_ListInitHead(S_ListHead_S* list)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(list != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- 创建链表表头 */
    list->prev = list;
    list->next = list;
}

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
void S_ListAdd(S_ListHead_S* new, S_ListHead_S* prev, S_ListHead_S* next)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(new != SCP_NULL);
    SCP_ASSERT(prev != SCP_NULL);
    SCP_ASSERT(next != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- 插入新节点 */
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

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
void S_ListAddHead(S_ListHead_S* new, S_ListHead_S* head)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(new != SCP_NULL);
    SCP_ASSERT(head != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- 链表头部插入新节点 */
    S_ListAdd(new, head, head->next);
}

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
void S_ListAddTail(S_ListHead_S* new, S_ListHead_S* head)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(new != SCP_NULL);
    SCP_ASSERT(head != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- 链表尾部插入新节点 */
    S_ListAdd(new, head->prev, head);
}

/**
 * @brief 在链表中删除某一节点
 * - [iSCP-SLib-SwCD-F-1815]
 *
 * @param[in] entry 要删除的结点的指针域
 *
 * @retval void 无
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
 */
void S_ListDel(S_ListHead_S* entry)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(entry != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- 删除某一节点 */
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = (void *) 0;
    entry->prev = (void *) 0;
}

/**
 * @brief 判断双向链表是否为空
 * - [iSCP-SLib-SwCD-F-1816]
 *
 * @param[in] head 链表表头的指针域
 *
 * @return bIsEmpty
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
 */
bool_t S_ListIsEmpty(const S_ListHead_S* head)
{
    /*- 变量初始化 */
    bool_t bIsEmpty = SCP_TRUE;
    
    /*- 判断链表头是否存在 */
    if (head != SCP_NULL)
    {
        /*- 判断链表是否只有一个节点 */
        if (head->next != head)
        {
            bIsEmpty = SCP_FALSE;
        }
    }
    return bIsEmpty;
}

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
void S_ListMoveHead(S_ListHead_S* node, S_ListHead_S* head)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(node != SCP_NULL);
    SCP_ASSERT(head != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- 从A链表删除后头插法插入B链表 */
    node->next->prev = node->prev;
    node->prev->next = node->next;
    S_ListAddHead(node, head);
}

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
void S_ListMoveTail(S_ListHead_S* node, S_ListHead_S* head)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(node != SCP_NULL);
    SCP_ASSERT(head != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- A链表删除后尾插法插入B链表 */
    node->next->prev = node->prev;
    node->prev->next = node->next;
    S_ListAddTail(node, head);
}

/**
 * @brief 计算链表节点数
 * - [iSCP-SLib-SwCD-F-1819]
 *
 * @param[in] head 链表表头的指针域
 *
 * @return 返回链表上节点数目
 * @see [iSCP-MPU-SwAD-F-0311] [iSCP-ATO-SwAD-F-0048]
*/
uint32_t S_ListCalcCount(const S_ListHead_S* head)
{
    /*- 变量初始化 */
    S_ListHead_S *pos;
    uint32_t ulCount = 0u;

    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(head != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    /*- 计算链表节点数 */
    S_LIST_FOR_EACH(pos, head)
    {
        ulCount++;
    }
    return ulCount;
}
