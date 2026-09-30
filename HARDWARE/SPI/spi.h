/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
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
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN Private defines */
/* begin modified by deng 2023.11.6 */
#define HAL_SPI_MAX_DELAY 150 //SPI数据收发最大延时
/* end modified by deng 2023.11.6  */
/* USER CODE END Private defines */

void MX_SPI2_Init(void);

/* USER CODE BEGIN Prototypes */
int32_t SPI_Mutex_Init(void);
int32_t HAL_SendSpeedAndReceivePWM(uint8_t* pucTxData, uint8_t* pucRxData, uint32_t ulSize);
/* begin modified by deng 2023.11.6 */
int32_t HAL_SendSpeed(uint8_t* pucTxData, uint32_t ulSize);
int32_t HAL_ReceivePWM(uint8_t* pucRxData, uint32_t ulSize);
/* end modified by deng 2023.11.6  */
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

