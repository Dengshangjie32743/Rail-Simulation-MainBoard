/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

extern FDCAN_HandleTypeDef hfdcan2;

/* USER CODE BEGIN Private defines */
#define CAN_DATA_LEN    8u
typedef struct {
    uint32_t ulCanId;
    uint32_t ulLen;
    uint8_t aucData[CAN_DATA_LEN];
} CAN_Frame_S;//can÷°∏Ò Ω
/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
int32_t HAL_SendIOMsg(uint32_t ulID, uint8_t *pucTxData, uint32_t ulSize);
int32_t HAL_ReceiveIOMsg(uint8_t *ulID, uint8_t *pucRxData, uint8_t *ucRet);
int32_t HAL_SendBTMMsg(uint32_t ulID, uint8_t *pucTxData, uint32_t ulSize);
int32_t HAL_ReceiveBTMMsg(uint32_t* ulID, uint8_t* pucRxData, uint32_t* ulSize);
int32_t CAN_Recv(uint32_t ulCanNum, CAN_Frame_S* pstCanFrame);
int32_t CAN_Send(uint32_t ulCanNum, CAN_Frame_S* pstCanFrame);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

