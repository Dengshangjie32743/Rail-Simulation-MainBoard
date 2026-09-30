/**
 * @brief 类型定义文件
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author
 * @file   TypeInc.h
 * @ingroup SLib
 *
 * @date 2022-01-20   任智立：首次创建
 *
 */

#ifndef __SLOG_H
#define __SLOG_H

#ifdef __cplusplus
extern "C" {
#endif
/* begin modified by deng 2023.11.6 */
#include "TypeInc.h"
/* end modified by deng 2023.11.6  */
/* begin add by deng 2024年12月1日 */
#include "udp_socket.h"
/* end add by deng 2024年12月1日 */

/*
 * 日志等级
 */
#define LOG_FATAL    (0u)
#define LOG_ERROR    (1u)
#define LOG_WARNING  (2u)
#define LOG_INFO     (3u)
#define LOG_DEBUG    (4u)


#define LOG_MOD_MAIN        0x00u
#define LOG_MOD_PRO         0x01u
#define LOG_MOD_BTM         0x02u
#define LOG_MOD_RADAR       0x03u
#define LOG_MOD_CL          0x04u
#define LOG_MOD_PWM         0x05u
#define LOG_MOD_SPEED       0x06u
/* begin modified by deng 2023.11.6 */
#define LOG_MOD_IO          0x07u
#define LOG_MOD_LED         0x08u
#define LOG_MOD_Init        0x09u
/* end modified by deng 2023.11.6  */
#define LOG_MOD_UDP         0x0Au
#define LOG_MOD_CPU_USAGE   0x0Bu
#define LOG_MAX_MODULE_ID   0x10u               /**< 最大模块Id */

int32_t LOG_Init(void);
void LOG(uint8_t ucLevel, uint8_t ucModuleId, const char *format, ...);
void LOG_BIN(uint8_t ucLevel, uint8_t ucModuleId, const uint8_t* pLogBin, uint32_t ulBinLen, const char *format, ...);
void LOG_LOOP(SOCKET_ADDR stSPSockaddr);

#ifdef __cplusplus
}
#endif

#endif
