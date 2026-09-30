/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
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
#include "spi.h"
#include "Use_Task.h"
#include "Log.h"

/* USER CODE BEGIN 0 */
#include "Smb.h"
/* USER CODE END 0 */

SPI_HandleTypeDef hspi2;

/* SPI2 init function */
void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
    /* begin modified by deng 2023年11月30日 */
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
/* begin modified by deng 2023年11月30日 */
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;    //用于PWM和速度计
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
/* end modified by deng 2024年1月30日 */
  hspi2.Init.NSS = SPI_NSS_SOFT;
/* begin modified by deng 2023年11月30日 */
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;//780kbitS/s(速度太快，接收信息有误 )
/* end modified by deng 2023年11月30日 */
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */
//  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0,GPIO_PIN_RESET);//SPI2_FPGA_CS_0;//将SPI2 CS引脚拉低  使能从设备
  /* USER CODE END SPI2_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */

  /** Initializes the peripherals clock
  */
        /* begin modified by deng 2023.10.24 */
//    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
//    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
    PeriphClkInitStruct.Spi2ClockSelection = RCC_SPI2CLKSOURCE_PLL;
    /* end modified by deng 2023.10.24 */
    
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOI_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PI1     ------> SPI2_SCK
    PI2     ------> SPI2_MISO
    PI3     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PI1     ------> SPI2_SCK
    PI2     ------> SPI2_MISO
    PI3     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/* begin modified by deng 2023.11.6 */
/*
 * 发送速度（控制FPGA）
 */
int32_t HAL_SendSpeed(uint8_t* pucTxData, uint32_t ulSize)
{
    int32_t lErrorCode = SCP_OK;
//    HAL_StatusTypeDef lRet;
/* begin add by deng 2023年11月30日 */
    uint8_t pucRxData[100] = {0u}; //无用
/* end add by deng 2023年11月30日 */
    OS_CPU_SR cpu_sr;

    if (pucTxData == NULL || ulSize == 0U)
    {
        lErrorCode = SCP_ERROR;
    }

/* begin modified by deng 2024年11月30日 */
    OS_ENTER_CRITICAL();      //关中断
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0,GPIO_PIN_RESET);//SPI2_FPGA_CS_0;//将SPI2的CS引脚拉低，使能从设备
//    lErrorCode = HAL_SPI_Transmit(&hspi2, pucTxData, ulSize, HAL_SPI_MAX_DELAY);
    lErrorCode = HAL_SPI_TransmitReceive(&hspi2, pucTxData, pucRxData, ulSize, HAL_SPI_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0,GPIO_PIN_SET);//SPI2_FPGA_CS_1;//将SPI2 CS引脚拉高  不使能从设备
    OS_EXIT_CRITICAL();       //开中断
/* end modified by deng 2024年11月30日 */

    if (lErrorCode != HAL_OK)
    {
        lErrorCode = SCP_ERROR;
    }

    return lErrorCode;
}

/*
 * 接收PWM（控制FPGA）
 */
int32_t HAL_ReceivePWM(uint8_t* pucRxData, uint32_t ulSize)
{
    int32_t lErrorCode = SCP_OK;
    OS_CPU_SR cpu_sr;

    if (pucRxData == NULL || ulSize == 0U)
    {
        lErrorCode = SCP_ERROR;
    }

/* begin modified by deng 2024年11月30日 */
    OS_ENTER_CRITICAL();      //关中断
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0,GPIO_PIN_RESET);//SPI2_FPGA_CS_0;//将SPI2的CS引脚拉低，使能从设备
    HAL_SPI_Receive(&hspi2, pucRxData, ulSize, HAL_SPI_MAX_DELAY);//SPI采集
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0,GPIO_PIN_SET);//SPI2_FPGA_CS_1;//将SPI2 CS引脚拉高  不使能从设备
    OS_EXIT_CRITICAL();       //开中断
/* end modified by deng 2024年11月30日 */

    if( lErrorCode != SCP_OK)
    {
        lErrorCode = SCP_ERROR;
    }

    return lErrorCode;
}
/* end modified by deng 2023.11.6  */
/* USER CODE END 1 */
