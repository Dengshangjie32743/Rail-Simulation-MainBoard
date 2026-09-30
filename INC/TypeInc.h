/**
 * @brief 类型定义文件
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author
 * @file   TypeInc.h
 * @ingroup Plt
 *
 * @date 2022-01-20   任智立：首次创建
 *
 */

#ifndef __TYPEINC_T
#define __TYPEINC_T

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "lwip/inet.h"

/* exact-width signed integer types */
#ifndef _CHAR_T_DECLARED
typedef                   char char_t;
#define _CHAR_T_DECLARED
#endif

#ifndef _INT8_T_DECLARED
typedef   signed          char int8_t;
#define _INT8_T_DECLARED
#endif

#ifndef _INT16_T_DECLARED
typedef   signed short     int int16_t;
#define _INT16_T_DECLARED
#endif

#ifndef _INT32_T_DECLARED
typedef   signed           int int32_t;
#define _INT32_T_DECLARED
#endif

#ifndef _INT64_T_DECLARED
#ifndef __COM_BOARD__
typedef   signed       long long int64_t;
#endif
#define _INT64_T_DECLARED
#endif


/* exact-width unsigned integer types */

#ifndef _UINT8_T_DECLARED
typedef unsigned          char uint8_t;
#define _UINT8_T_DECLARED
#endif

#ifndef _UINT16_T_DECLARED
typedef unsigned short     int uint16_t;
#define _UINT16_T_DECLARED
#endif

#ifndef _UINT32_T_DECLARED
typedef unsigned           int uint32_t;
#define _UINT32_T_DECLARED
#endif

#ifndef _UINT64_T_DECLARED
#ifndef __COM_BOARD__
typedef unsigned       long long  uint64_t;
#endif
#define _UINT64_T_DECLARED
#endif


typedef float   fp32_t;
typedef double  fp64_t;

#define STATIC static

typedef uint32_t    bool_t;
#define SCP_TRUE    (0xFFFFFFFF)
#define SCP_FALSE   (0)

#define SCP_NULL    ((void *)0)

#define SCP_OK      (0)
#define SCP_ERROR   (-1)

#if 0
/* minimum values of exact-width signed integer types */
#define INT8_MIN                   -128
#define INT16_MIN                -32768
#define INT32_MIN          (~0x7fffffff)   /* -2147483648 is unsigned */
#define INT64_MIN  (~0x7fffffffffffffff) /* -9223372036854775808 is unsigned */

/* maximum values of exact-width signed integer types */
#define INT8_MAX                    127
#define INT16_MAX                 32767
#define INT32_MAX            2147483647
#define INT64_MAX            (9223372036854775807)

/* maximum values of exact-width unsigned integer types */
#define UINT8_MAX                   255
#define UINT16_MAX                65535
#define UINT32_MAX           4294967295u
#define UINT64_MAX           (18446744073709551615)
#endif


#define TRY_BEGIN   int32_t __return_code = SCP_OK;int32_t __error_line = 0;(void)__return_code;(void)__error_line;
#define TRY_END     __gotoErrorCode:
#define THROW(ret)  do{__return_code = (ret); __error_line = __LINE__; goto __gotoErrorCode;}while(0)
#define ERROR_CODE  __return_code
#define ERROR_LINE  __error_line

#define JUDGE(expr)             do{if (!(expr)) {THROW(SCP_ERROR);}}while(0)
#define JUDGE_EX(errcode, expr) do{if (!(expr)) {THROW(errcode);}}while(0)
#define EXEC(func)              do{__return_code = (func); if (SCP_OK != __return_code) {THROW(__return_code);}}while(0)
#define EXEC_EX(errcode, func)  do{if (SCP_OK != (func)) {THROW(errcode);}}while(0)

#define SCP_ASSERT(expr) do{if (!(expr)) {while(1);}}while(0);
#define Error_Handler() while(1) //驱动采用的宕机函数

#ifdef __cplusplus
}
#endif

#endif
