#include "usart.h"
#include "main.h"
#include <stdio.h>

/* 私有变量定义 */
UART_HandleTypeDef UartHandle;
uint8_t aTxBuffer[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
uint8_t aRxBuffer[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/* printf重定向缓冲区 */
#define PRINTF_BUFFER_SIZE 128
static char printf_buffer[PRINTF_BUFFER_SIZE];
static uint8_t printf_index = 0;
static void Error_Handler();

/**
  * @brief  printf重定向函数
  * @param  ch: 要发送的字符
  * @retval int: 发送的字符数
  */
int fputc(int ch, FILE *f)
{
    if (printf_index < PRINTF_BUFFER_SIZE - 1)
    {
        printf_buffer[printf_index++] = ch;
        
        /* 遇到换行符或缓冲区满时发送数据 */
        if (ch == '\n' || printf_index >= PRINTF_BUFFER_SIZE - 1)
        {
            printf_buffer[printf_index] = '\0';
            HAL_UART_Transmit_IT(&UartHandle, (uint8_t *)printf_buffer, printf_index);
            printf_index = 0;
        }
    }
    return ch;
}

/**
  * @brief  USART配置函数
  * @param  无
  * @retval 无
  */
void USART_Config(void)
{
  /* USART1初始化 */
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  UartHandle.Instance          = USART1;
  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  if (HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  USART错误回调执行函数，输出错误代码
  * @param  huart: UART句柄
  * @retval 无
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  printf("Uart Error, ErrorCode = %d\r\n", huart->ErrorCode);
}

/**
  * @brief  USART发送回调执行函数
  * @param  UartHandle: UART句柄
  * @retval 无
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /*通过中断方式接收数据*/
  if (HAL_UART_Receive_IT(UartHandle, (uint8_t *)aRxBuffer, 12) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  USART接收回调执行函数
  * @param  UartHandle: UART句柄
  * @retval 无
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /*通过中断方式接收数据*/
  if (HAL_UART_Transmit_IT(UartHandle, (uint8_t *)aRxBuffer, 12) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  启动串口传输
  * @param  无
  * @retval HAL状态
  */
HAL_StatusTypeDef USART_StartTransmission(void)
{
  return HAL_UART_Transmit_IT(&UartHandle, (uint8_t *)aTxBuffer, 12);
}

/**
  * @brief  获取UART句柄
  * @param  无
  * @retval UART句柄指针
  */
UART_HandleTypeDef* USART_GetHandle(void)
{
  return &UartHandle;
}

/**
  * @brief  直接发送字符串
  * @param  str: 要发送的字符串
  * @retval HAL状态
  */
HAL_StatusTypeDef USART_SendString(const char *str)
{
    uint16_t len = strlen(str);
    return HAL_UART_Transmit_IT(&UartHandle, (uint8_t *)str, len);
}

static void Error_Handler()
{
  while (1);
}