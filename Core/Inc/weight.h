#ifndef __WEIGHT_H
#define __WEIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含文件 */
#include "py32f0xx_hal.h"
#include "cs1237.h"
#include "main.h"

/* 函数声明 */
HAL_StatusTypeDef Weight_Init(CS1237_HandleTypeDef *hcs1237);
HAL_StatusTypeDef Weight_Read(float *weight);
HAL_StatusTypeDef Weight_Calibrate(void);
HAL_StatusTypeDef Weight_SetFactor(float factor);
HAL_StatusTypeDef Weight_SetOffset(float offset);
uint8_t Weight_GetGain(void);
uint16_t Weight_GetSpeed(void);
CS1237_HandleTypeDef* Weight_GetHandle(void);

// New function declarations
HAL_StatusTypeDef Weight_Update(void);
int32_t Weight_ConvertAdcToWeight(int32_t adc_value);
int32_t Weight_GetFilteredWeight(void);
int32_t Weight_GetCurrentWeight(void);
uint8_t Weight_IsLimitExceeded(void);
HAL_StatusTypeDef Weight_SetLimit(int32_t limit);

extern   CS1237_HandleTypeDef hcs1237;

#ifdef __cplusplus
}
#endif

#endif /* __WEIGHT_H */ 