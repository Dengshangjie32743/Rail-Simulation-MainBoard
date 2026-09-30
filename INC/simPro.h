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
#ifndef SIMPRO_H_
#define SIMPRO_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "TypeInc.h"
#include "udp_socket.h"

void SP_Init(void);
int32_t SP_Process(uint8_t *pucData, uint32_t ulDataLen, SOCKET_ADDR stSockaddr);
void SP_Loop(void);

#ifdef __cplusplus
}
#endif

#endif /* SIMPRO_H_ */


