#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////     
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32H7开发板
//LED驱动代码       
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2017/6/8
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved                                      
//////////////////////////////////////////////////////////////////////////////////     

//LED端口定义
#define LED0(n)        (n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_RESET))
#define LED0_Toggle    (HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2)) //LED0输出电平翻转
#define LED1(n)        (n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,GPIO_PIN_RESET))
#define LED1_Toggle    (HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3)) //LED1输出电平翻转

#define LED2(n)        (n?HAL_GPIO_WritePin(GPIOI,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOI,GPIO_PIN_6,GPIO_PIN_RESET))
#define LED2_Toggle    (HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_6)) //LED2输出电平翻转
#define LED3(n)        (n?HAL_GPIO_WritePin(GPIOI,GPIO_PIN_7,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOI,GPIO_PIN_7,GPIO_PIN_RESET))
#define LED3_Toggle    (HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_7)) //LED3输出电平翻转

void LED_Init(void); //LED初始化函数
#endif
