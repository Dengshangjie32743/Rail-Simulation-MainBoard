/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "Smb.h"
/* USER CODE END 0 */

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart6;

bool_t g_IsSending = SCP_FALSE; // 串口发送状态标志

/* UART4 init function */
void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.Prescaler = UART_PRESCALER_DIV1;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN UART4_Init 2 */
  HAL_NVIC_SetPriority(UART4_IRQn, 1, 5); // 设置优先级
  HAL_NVIC_EnableIRQ(UART4_IRQn); // 使能中断
  /* USER CODE END UART4_Init 2 */

}
/* USART6 init function */

/* begin modified by deng 2023.11.6 */
void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.Init.Prescaler = UART_PRESCALER_DIV1;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);//使能RS422控制引脚发送
  /* USER CODE END USART6_Init 2 */

}
/* end modified by deng 2023.11.6  */

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4;
    PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* UART4 clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();

    __HAL_RCC_GPIOH_CLK_ENABLE();
    /**UART4 GPIO Configuration
    PH13     ------> UART4_TX
    PH14     ------> UART4_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspInit 0 */

  /* USER CODE END USART6_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
    PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART6 clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN USART6_MspInit 1 */

  /* USER CODE END USART6_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();

    /**UART4 GPIO Configuration
    PH13     ------> UART4_TX
    PH14     ------> UART4_RX
    */
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_13|GPIO_PIN_14);

  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspDeInit 0 */

  /* USER CODE END USART6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART6_CLK_DISABLE();

    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN USART6_MspDeInit 1 */

  /* USER CODE END USART6_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/*
 * 发RS485数据(Baud rate = 115200)
 */
uint32_t HAL_SendRadarMsg(uint8_t *pucData, uint32_t ulSize)
{
    int32_t lErrorCode = SCP_OK;
    bool_t bIsSending;//发送状态标志位

    if (pucData == NULL || ulSize == 0U)
    {
        // 返回错误代码，表示数据指针为空或数据长度为0
        lErrorCode = SCP_ERROR;
    }

/* begin modified by deng 2024年12月1日 */
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);//拉高RS485控制引脚电平
//    if(HAL_UART_Transmit(&huart4, pucData, ulSize, HAL_RADAR_DELAY) != SCP_OK)
//    {
//        lErrorCode = SCP_ERROR;
//    }
    HAL_NVIC_DisableIRQ(EXTI0_IRQn); // 关中断
    bIsSending = g_IsSending; // 获取当前发送状态
    HAL_NVIC_EnableIRQ(EXTI0_IRQn); // 开中断
    if (!bIsSending)
    {
        bIsSending = SCP_TRUE; // 设置发送状态为正在发送
        HAL_UART_Transmit_IT(&huart4, pucData, ulSize); // 使用中断发送
        g_IsSending = bIsSending; // 更新全局发送状态
    }
/* end modified by deng 2024年12月1日 */

    return lErrorCode;
}

/* begin modified by deng 2023.11.6 */
/*
 *功能：重定向printf
 */
int32_t fputc(int32_t lCh, FILE *f)
{
/* 将Printf内容发往串口 */
  HAL_UART_Transmit(&huart6, (void*)&lCh, 1, 1000);
  return (lCh);
}
/* end modified by deng 2023.11.6  */
/* begin add by deng 2024年12月1日 */
//UART4中断服务函数
void UART4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart4);
}
// UART中断回调函数
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (UART4 == huart->Instance)
    {
        HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
        g_IsSending = SCP_FALSE; // 发送完成，重置状态
        HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断
    }
}
/* end add by deng 2024年12月1日 */
/* USER CODE END 1 */
