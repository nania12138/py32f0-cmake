/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "cs1237.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "py32f0xx_hal.h"
#include "command.h"
#include "weight.h"
#include "main.h"
#include "usart.h"
#include "app.h"

/* Private define ------------------------------------------------------------*/
#define SCK_PIN_0 GPIO_PIN_5
#define SCK_PORT_0 GPIOA
#define DOUT_PIN_0 GPIO_PIN_7
#define DOUT_PORT_0 GPIOA

#define PROBE_OUT_PIN GPIO_PIN_0
#define PROBE_OUT_PORT GPIOA
#define PROBE_LEV_PIN GPIO_PIN_1
#define PROBE_LEV_PORT GPIOA
#define PROBE_GAN_PIN GPIO_PIN_4
#define PROBE_GAN_PORT GPIOA

#define LED_PIN GPIO_PIN_3
#define LED_PORT GPIOB
#define PULSE_PIN GPIO_PIN_6
#define PULSE_PORT GPIOA
#define BUTTON_PIN GPIO_PIN_6
#define BUTTON_PORT GPIOB

/* Private variables ---------------------------------------------------------*/


UART_HandleTypeDef huart1;

/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_GpioConfig(void);
static void APP_UARTConfig(void);


/**
  * @brief  应用程序入口函数.
  * @retval int
  */
int main(void)
{
  /* 初始化所有外设，Flash接口，SysTick */
  HAL_Init();

  /* 初始化应用程序 */
  // 初始化GPIO
  APP_SystemClockConfig();
  APP_GpioConfig();

  /* 等待CS1237芯片稳定 */
  HAL_Delay(500);  // 保持毫秒级延时用于初始化
  // 初始化UART
  APP_UARTConfig();
  APP_Init();

  /* 主循环 */
  while (1)
  {
      APP_Process();
      HAL_Delay(1); // 1ms delay
  }
}

static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                            /* Turn on HSE */
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                       /* HSE frequency range */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                        /* PLL ON */
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;                /* PLL clock source from HSE (freq >= 12MHz) */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;           /* Set PLL as SYSCLK source */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                  /* APH no division */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                   /* APB no division */
  /* 
   * Re-initialize RCC clock
   * -- clock <= 24MHz: FLASH_LATENCY_0
   * -- clock > 24MHz:  FLASH_LATENCY_1
   */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}


/**
  * @brief  GPIO配置
  * @param  无
  * @retval 无
  */
static void APP_GpioConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 使能GPIO时钟 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* 配置输出引脚 */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStruct.Pin = PROBE_OUT_PIN;
  HAL_GPIO_Init(PROBE_OUT_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PROBE_OUT_PORT, PROBE_OUT_PIN, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = LED_PIN;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = PULSE_PIN;
  HAL_GPIO_Init(PULSE_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PULSE_PORT, PULSE_PIN, GPIO_PIN_RESET);

  /* 配置输入引脚 */
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;

  GPIO_InitStruct.Pin = PROBE_LEV_PIN;
  HAL_GPIO_Init(PROBE_LEV_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = PROBE_GAN_PIN;
  HAL_GPIO_Init(PROBE_GAN_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BUTTON_PIN;
  HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
}

/**
  * @brief  UART配置
  * @param  无
  * @retval 无
  */
static void APP_UARTConfig(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  错误执行函数
  * @param  无
  * @retval 无
  */
void APP_ErrorHandler(void)
{
    while (1);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  输出产生断言错误的源文件名及行号
  * @param  file：源文件名指针
  * @param  line：发生断言错误的行号
  * @retval 无
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    while (1);
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
