/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
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
#include "fdcan.h"

/* USER CODE BEGIN 0 */
#include "Smb.h"
/* begin add by deng 2023年11月30日 */
#include "QueueInc.h"
extern SMB_Queue_S g_astInputIOData;
extern SMB_Queue_S g_astBTM0Data;
extern SMB_Queue_S g_astBTM1Data;
/* end add by deng 2023年11月30日 */
/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;
/* begin add by deng 2023年11月30日 */
FDCAN_RxHeaderTypeDef FDCAN1_RxHeader;
FDCAN_RxHeaderTypeDef FDCAN2_RxHeader;
/* end add by deng 2023年11月30日 */

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */
  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */
  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 5;
  hfdcan1.Init.NominalSyncJumpWidth = 8;
  hfdcan1.Init.NominalTimeSeg1 = 5;
  hfdcan1.Init.NominalTimeSeg2 = 4;
  hfdcan1.Init.DataPrescaler = 5;
  hfdcan1.Init.DataSyncJumpWidth = 4;
  hfdcan1.Init.DataTimeSeg1 = 5;
  hfdcan1.Init.DataTimeSeg2 = 4;
  /* begin modified by deng 2023年11月30日 */
//  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.MessageRAMOffset = 64;//设置与fdcan2的起始地址不同
/* end modified by deng 2023年11月30日 */
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 1;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 2;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  /* begin modified by deng 2023年11月30日 */
//  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_64;//发送区扩大至64字节
/* end modified by deng 2023年11月30日 */
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* Start the FDCAN module */
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }

/* begin add by deng 2023年11月30日 */
  HAL_FDCAN_ActivateNotification(&hfdcan1,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);//激活FIFO0新消息中断
/* end add by deng 2023年11月30日 */
  /* USER CODE END FDCAN1_Init 2 */

}
/* FDCAN2 init function */
void MX_FDCAN2_Init(void)
{

    /* USER CODE BEGIN FDCAN2_Init 0 */
    FDCAN_FilterTypeDef FDCAN2_RXFilter;
    /* USER CODE END FDCAN2_Init 0 */

    /* USER CODE BEGIN FDCAN2_Init 1 */

    /* USER CODE END FDCAN2_Init 1 */
    hfdcan2.Instance = FDCAN2;
    hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan2.Init.AutoRetransmission = ENABLE;
    hfdcan2.Init.TransmitPause = DISABLE;
    hfdcan2.Init.ProtocolException = ENABLE;
    hfdcan2.Init.NominalPrescaler = 5;
    hfdcan2.Init.NominalSyncJumpWidth = 8;
    hfdcan2.Init.NominalTimeSeg1 = 5;
    hfdcan2.Init.NominalTimeSeg2 = 4;
    hfdcan2.Init.DataPrescaler = 5;
    hfdcan2.Init.DataSyncJumpWidth = 4;
    hfdcan2.Init.DataTimeSeg1 = 5;
    hfdcan2.Init.DataTimeSeg2 = 4;
    hfdcan2.Init.MessageRAMOffset = 0;//设置与fdcan1的起始地址不同
    hfdcan2.Init.StdFiltersNbr = 1;
    hfdcan2.Init.ExtFiltersNbr = 0;
    /* begin deleted by deng 2023年11月30日 */
//    hfdcan2.Init.RxFifo0ElmtsNbr = 0;
//    hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
    /* end deleted by deng 2023年11月30日 */
    hfdcan2.Init.RxFifo1ElmtsNbr = 1;
    hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.RxBuffersNbr = 1;
    hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.TxEventsNbr = 0;
    hfdcan2.Init.TxBuffersNbr = 0;
    hfdcan2.Init.TxFifoQueueElmtsNbr = 2;
    hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
    if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
    {
      Error_Handler();
    }

    //配置can过滤器
    FDCAN2_RXFilter.IdType=FDCAN_STANDARD_ID;                       //标准ID
    FDCAN2_RXFilter.FilterIndex=0;                                  //滤波器索????????
    FDCAN2_RXFilter.FilterType=FDCAN_FILTER_MASK;                   //滤波器类????????
    FDCAN2_RXFilter.FilterConfig=FDCAN_FILTER_TO_RXFIFO1;           //过滤????????0关联到FIFO0
    FDCAN2_RXFilter.FilterID1=0x0000;                               //32位ID
    FDCAN2_RXFilter.FilterID2=0x0000;                               //如果FDCAN配置为传统模式的话，这里????????32位掩????????
    HAL_FDCAN_ConfigFilter(&hfdcan2,&FDCAN2_RXFilter);              //滤波器初始化

    /* USER CODE BEGIN FDCAN2_Init 2 */
    HAL_FDCAN_Start(&hfdcan2);                               //启FDCAN2
    HAL_FDCAN_ActivateNotification(&hfdcan2,FDCAN_IT_RX_FIFO1_NEW_MESSAGE,1);//激活FIFO1新消息中断
    /* USER CODE END FDCAN2_Init 2 */

}

static uint32_t HAL_RCC_FDCAN_CLK_ENABLED=0;

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(HAL_RCC_FDCAN_CLK_ENABLED==1){
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN FDCAN1_MspInit 1 */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn,1,2);//设置中断优先级
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);//使能中断
  /* USER CODE END FDCAN1_MspInit 1 */
  }
  else if(fdcanHandle->Instance==FDCAN2)
  {
  /* USER CODE BEGIN FDCAN2_MspInit 0 */

  /* USER CODE END FDCAN2_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN2 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(HAL_RCC_FDCAN_CLK_ENABLED==1){
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**FDCAN2 GPIO Configuration
    PB12     ------> FDCAN2_RX
    PB13     ------> FDCAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN FDCAN2_MspInit 1 */
    /* begin add by deng 2023年11月30日 */
    HAL_NVIC_SetPriority(FDCAN2_IT0_IRQn,1,4);//设置中断优先级
    HAL_NVIC_EnableIRQ(FDCAN2_IT0_IRQn);//使能中断
    /* end add by deng 2023年11月30日 */
  /* USER CODE END FDCAN2_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_FDCAN_CLK_ENABLED--;
    if(HAL_RCC_FDCAN_CLK_ENABLED==0){
      __HAL_RCC_FDCAN_CLK_DISABLE();
    }

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
  else if(fdcanHandle->Instance==FDCAN2)
  {
  /* USER CODE BEGIN FDCAN2_MspDeInit 0 */

  /* USER CODE END FDCAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_FDCAN_CLK_ENABLED--;
    if(HAL_RCC_FDCAN_CLK_ENABLED==0){
      __HAL_RCC_FDCAN_CLK_DISABLE();
    }

    /**FDCAN2 GPIO Configuration
    PB12     ------> FDCAN2_RX
    PB13     ------> FDCAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

  /* USER CODE BEGIN FDCAN2_MspDeInit 1 */
/* begin add by deng 2023年11月30日 */
    HAL_NVIC_DisableIRQ(FDCAN2_IT0_IRQn);
/* end add by deng 2023年11月30日 */
  /* USER CODE END FDCAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/*
 * 发送CAN帧的长度转换
 */
STATIC uint32_t HAL_GetFDCanDlcBytesMacro(uint32_t ulbytes)
{
    switch (ulbytes)
    {
        case 0U:
                return FDCAN_DLC_BYTES_0;
        case 1U:
                return FDCAN_DLC_BYTES_1;
        case 2U:
                return FDCAN_DLC_BYTES_2;
        case 3U:
                return FDCAN_DLC_BYTES_3;
        case 4U:
                return FDCAN_DLC_BYTES_4;
        case 5U:
                return FDCAN_DLC_BYTES_5;
        case 6U:
                return FDCAN_DLC_BYTES_6;
        case 7U:
                return FDCAN_DLC_BYTES_7;
        case 8U:
                return FDCAN_DLC_BYTES_8;
        case 12U:
                return FDCAN_DLC_BYTES_12;
        case 16U:
                return FDCAN_DLC_BYTES_16;
        case 20U:
                return FDCAN_DLC_BYTES_20;
        case 24U:
                return FDCAN_DLC_BYTES_24;
        case 32U:
                return FDCAN_DLC_BYTES_32;
        case 48U:
                return FDCAN_DLC_BYTES_48;
        case 64U:
                return FDCAN_DLC_BYTES_64;
        default:
                return (uint32_t)SCP_ERROR;
    }
}
/*
 * 接收CAN帧的长度转换
 */
STATIC uint32_t HAL_GetFDCanDlcBytesReverse(uint32_t uDlc)
{
    switch (uDlc)
    {
        case FDCAN_DLC_BYTES_0:
            return 0U;
        case FDCAN_DLC_BYTES_1:
            return 1U;
        case FDCAN_DLC_BYTES_2:
            return 2U;
        case FDCAN_DLC_BYTES_3:
            return 3U;
        case FDCAN_DLC_BYTES_4:
            return 4U;
        case FDCAN_DLC_BYTES_5:
            return 5U;
        case FDCAN_DLC_BYTES_6:
            return 6U;
        case FDCAN_DLC_BYTES_7:
            return 7U;
        case FDCAN_DLC_BYTES_8:
            return 8U;
        case FDCAN_DLC_BYTES_12:
            return 12U;
        case FDCAN_DLC_BYTES_16:
            return 16U;
        case FDCAN_DLC_BYTES_20:
            return 20U;
        case FDCAN_DLC_BYTES_24:
            return 24U;
        case FDCAN_DLC_BYTES_32:
            return 32U;
        case FDCAN_DLC_BYTES_48:
            return 48U;
        case FDCAN_DLC_BYTES_64:
            return 64U;
        default:
            return (uint32_t)SCP_ERROR;
    }
}

/*
 * 发送IO数据(FDCAN波特率 = 500k)
 */
int32_t HAL_SendIOMsg(uint32_t ulID, uint8_t *pucTxData, uint32_t ulSize)
{
    FDCAN_TxHeaderTypeDef FDCAN_TxHeader;
    int32_t lErrorCode = SCP_OK;

    if (pucTxData == NULL || HAL_GetFDCanDlcBytesMacro(ulSize) == (uint32_t)SCP_ERROR)
   {
    lErrorCode = SCP_ERROR;
   }

    FDCAN_TxHeader.Identifier=ulID;
    FDCAN_TxHeader.IdType=FDCAN_STANDARD_ID;
    FDCAN_TxHeader.TxFrameType=FDCAN_DATA_FRAME;
    FDCAN_TxHeader.DataLength= HAL_GetFDCanDlcBytesMacro(ulSize);
    FDCAN_TxHeader.ErrorStateIndicator=FDCAN_ESI_ACTIVE;
    FDCAN_TxHeader.BitRateSwitch=FDCAN_BRS_OFF;
    FDCAN_TxHeader.FDFormat=FDCAN_FD_CAN;                     //FDCAN类型
    FDCAN_TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;
    FDCAN_TxHeader.MessageMarker=0;
    if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &FDCAN_TxHeader, pucTxData) != HAL_OK)
    {
        lErrorCode = SCP_ERROR;
    }
//    HAL_Delay(5); //can发送延时（由于can协议传输速率低）

    return lErrorCode;
}

/*
 * 接收IO数据(FDCAN波特率 = 500k) (RxFIFO0回调函数)
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t uNum;
    uint8_t aucRxdata[8];
    S_Node_S IOData;
    if((RxFifo0ITs&FDCAN_IT_RX_FIFO0_NEW_MESSAGE)!=RESET)   //FIFO1新数据中断
    {
        //提取FIFO0中接收到的数据
        if(HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0,&FDCAN1_RxHeader,aucRxdata) == SCP_OK)
        {
            IOData.ulCanSrcId = FDCAN1_RxHeader.Identifier;
            IOData.ulCanLength = HAL_GetFDCanDlcBytesReverse(FDCAN1_RxHeader.DataLength);
            for(uNum = 0; uNum<8; uNum++)IOData.aucData[uNum] = aucRxdata[uNum];
            HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
            S_EnQueue(&g_astInputIOData, IOData);//
            HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断
        }
        HAL_FDCAN_ActivateNotification(hfdcan,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);//激活FIFO0新消息中断
    }
}
/* end add by deng 2023年11月30日 */

/*
 * 发送BTM数据(CAN波特率 = 500k)
 */
int32_t HAL_SendBTMMsg(uint32_t ulID, uint8_t *pucTxData, uint32_t ulSize)
{
    FDCAN_TxHeaderTypeDef FDCAN_TxHeader;
    int32_t lErrorCode = HAL_OK;
    int32_t i = 10;

    if (pucTxData == NULL || ulSize == 0U)
    {
      lErrorCode = SCP_ERROR;
    }

    FDCAN_TxHeader.Identifier=ulID;
    FDCAN_TxHeader.IdType=FDCAN_STANDARD_ID;
    FDCAN_TxHeader.TxFrameType=FDCAN_DATA_FRAME;
    FDCAN_TxHeader.DataLength= ulSize << 16U;
    FDCAN_TxHeader.ErrorStateIndicator=FDCAN_ESI_ACTIVE;
    FDCAN_TxHeader.BitRateSwitch=FDCAN_BRS_OFF;
    FDCAN_TxHeader.FDFormat=FDCAN_CLASSIC_CAN;              //传统CAN类型
    FDCAN_TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;
    FDCAN_TxHeader.MessageMarker=0;

/* begin deleted by deng 2024年3月31日 */
//    if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &FDCAN_TxHeader, pucTxData)!=HAL_OK)
//    {
//      lErrorCode = SCP_ERROR;
//    }
//    HAL_Delay(1);//延时给总线空闲时间
/* end deleted by deng 2024年3月31日 */

/* begin add by Zhuchengyin 2024年3月31日 */
    lErrorCode = SCP_ERROR;
    while (i--)
    {
        if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &FDCAN_TxHeader, pucTxData) == HAL_OK)
        {
            lErrorCode = SCP_OK;
            break;
        } 
        HAL_Delay(1);
    }
/* end add by Zhuchengyin 2024年3月31日 */

/* begin deleted by deng 2023年11月30日 */
//    HAL_FDCAN_Stop(&hfdcan2);      //结束FDCAN2
/* end deleted by deng 2023年11月30日 */

    return lErrorCode;
}

/*
 * 接收BTM数据(CAN波特率 = 500k) (RxFIFO1回调函数)
 */
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    uint8_t uNum;
   uint8_t aucRxdata[8];
    S_Node_S BTMData;

    if((RxFifo1ITs&FDCAN_IT_RX_FIFO1_NEW_MESSAGE)!=RESET)   //FIFO1新数据中断
    {
        //提取FIFO0中接收到的数据
        if(HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO1,&FDCAN2_RxHeader,aucRxdata) == SCP_OK)
        {
            BTMData.ulCanSrcId = FDCAN2_RxHeader.Identifier;
            BTMData.ulCanLength = HAL_GetFDCanDlcBytesReverse(FDCAN2_RxHeader.DataLength);
            for(uNum = 0; uNum < 8; uNum++)BTMData.aucData[uNum] = aucRxdata[uNum];
            HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
            S_EnQueue(&g_astBTM0Data, BTMData);
            HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断
        }
       HAL_FDCAN_ActivateNotification(&hfdcan2,FDCAN_IT_RX_FIFO1_NEW_MESSAGE,1);//激活FIFO1新消息中断
    }

}

/* begin modified by deng 2023.11.6 */
/*
 * 两路发送BTM（todo:设置第二路can用于收发BTM）
 */
int32_t CAN_Send(uint32_t ulCanNum, CAN_Frame_S* pstCanFrame)
{
    int32_t lErrorCode = SCP_OK;

    if (ulCanNum == 0U) //btm0
    {
        HAL_SendBTMMsg(pstCanFrame->ulCanId, pstCanFrame->aucData, pstCanFrame->ulLen);
    }
    else if(ulCanNum == 1U) //btm1
    {
        HAL_SendBTMMsg(pstCanFrame->ulCanId, pstCanFrame->aucData, pstCanFrame->ulLen);
    }
    else
    {
        lErrorCode = SCP_ERROR;
    }

    return lErrorCode;
}

/*
 * 两路接收BTM（todo:设置第二路can用于收发BTM）
 */
int32_t CAN_Recv(uint32_t ulCanNum, CAN_Frame_S* pstCanFrame)
{
  int32_t lErrorCode = SCP_OK;
  /* begin add by deng 2023年11月30日 */
  uint8_t uNum;
  S_Node_S BTMData;

  if (ulCanNum == 0U) //btm0
  {
      //获取BTM0数据
      HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
      if(S_DeQueue(&g_astBTM0Data, &BTMData) == SCP_OK)//从队列取BTM数据
      {
          pstCanFrame->ulCanId = BTMData.ulCanSrcId;
          pstCanFrame->ulLen = BTMData.ulCanLength;
          for(uNum = 0; uNum < 8; uNum++) pstCanFrame->aucData[uNum] = BTMData.aucData[uNum];
      }
      HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断
  }
  else if(ulCanNum == 1U) //btm1
  {
      //获取BTM1数据
      HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关中断
      if(S_DeQueue(&g_astBTM1Data, &BTMData) == SCP_OK)//从队列取BTM数据
      {
          pstCanFrame->ulCanId = BTMData.ulCanSrcId;
          pstCanFrame->ulLen = BTMData.ulCanLength;
          for(uNum = 0; uNum < 8; uNum++) pstCanFrame->aucData[uNum] = BTMData.aucData[uNum];
      }
      HAL_NVIC_EnableIRQ(EXTI0_IRQn);   //开中断
  }
  else
  {
    lErrorCode = SCP_ERROR;
  }
/* end add by deng 2023年11月30日 */

  return lErrorCode;
}
/* end modified by deng 2023.11.6  */
/* begin add by deng 2023年11月30日 */
//FDCAN1中断服务函数
void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&hfdcan1);
}
//FDCAN2中断服务函数
void FDCAN2_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&hfdcan2);
}
/* end add by deng 2023年11月30日 */

/* USER CODE END 1 */
