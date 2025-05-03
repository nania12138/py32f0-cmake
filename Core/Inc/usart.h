#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含文件 */
#include "py32f0xx_hal.h"
#include <stdio.h>
#include <string.h>

/* 函数声明 */
void USART_Config(void);
HAL_StatusTypeDef USART_StartTransmission(void);
UART_HandleTypeDef* USART_GetHandle(void);
HAL_StatusTypeDef USART_SendString(const char *str);

/* 回调函数声明 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */ 