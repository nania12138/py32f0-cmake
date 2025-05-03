#include "weight.h"
#include "cs1237.h"
#include "usart.h"
#include <stdio.h>

/* 私有变量定义 */
static float weight_offset = 0.0f;
static float weight_factor = 1.0f;
CS1237_HandleTypeDef hcs1237;

// Weight measurement variables
static int32_t g_previousWeight = 0;
static int32_t g_weightLimit = 444;
static uint8_t g_isLimitExceeded = 0;
static int32_t g_currentWeight_0 = 0;

// Kalman filter variables
static double g_measurementError = 0;
static double g_estimateError = 0;
static int32_t g_estimatedWeight = 0;
static double g_kalmanGain = 0.0;
static const float g_alpha = 0.2;

/**
  * @brief  初始化称重系统
  * @param  hspi: SPI句柄
  * @param  cs_port: 片选端口
  * @param  cs_pin: 片选引脚
  * @retval HAL状态
  */
HAL_StatusTypeDef Weight_Init(CS1237_HandleTypeDef *hcs1237)
{
    /* 初始化CS1237 */
    if (CS1237_Init(hcs1237) != HAL_OK)
    {
        return HAL_ERROR;
    }
    hcs1237->gain = CS1237_GAIN_128;
    hcs1237->speed = CS1237_SPEED_10HZ;

    /* 设置默认增益和速率 */
    if (CS1237_UpdateConfig(hcs1237) == HAL_OK)
    {
        return HAL_ERROR;
    }
    
    /* 校准 */
    if (Weight_Calibrate() != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

/**
  * @brief  读取重量值
  * @param  weight: 重量值指针
  * @retval HAL状态
  */
HAL_StatusTypeDef Weight_Read(float *weight)
{
    int32_t raw_data;
    
    if (CS1237_ReadData(&hcs1237, &raw_data) != HAL_OK)
    {
        return HAL_ERROR;
    }
      /* 应用偏移和增益校正 */
    *weight = ((int32_t)raw_data - hcs1237.offset) * hcs1237.gain_factor;
    /* 转换为重量值 */
    *weight = (float)raw_data * weight_factor - weight_offset;
    
    return HAL_OK;
}

/**
  * @brief  校准称重系统
  * @param  无
  * @retval HAL状态
  */
HAL_StatusTypeDef Weight_Calibrate(void)
{
    int32_t sum = 0;
    const int samples = 10;
    
    /* 读取多次数据取平均值 */
    for (int i = 0; i < samples; i++)
    {
        int32_t data;
        if (CS1237_ReadData(&hcs1237, &data) != HAL_OK)
        {
            return HAL_ERROR;
        }
        sum += data;
        HAL_Delay(100);
    }
    
    /* 计算零点偏移 */
    weight_offset = (float)sum / samples;
    
    /* 设置默认重量因子 */
    weight_factor = 0.0001f;  // 根据实际传感器特性调整
    
    return HAL_OK;
}

/**
  * @brief  设置重量因子
  * @param  factor: 重量因子
  * @retval HAL状态
  */
HAL_StatusTypeDef Weight_SetFactor(float factor)
{
    if (factor <= 0.0f)
    {
        return HAL_ERROR;
    }
    
    weight_factor = factor;
    return HAL_OK;
}

/**
  * @brief  设置零点偏移
  * @param  offset: 零点偏移值
  * @retval HAL状态
  */
HAL_StatusTypeDef Weight_SetOffset(float offset)
{
    weight_offset = offset;
    return HAL_OK;
}

/**
  * @brief  获取当前增益设置
  * @param  无
  * @retval 增益值
  */
uint8_t Weight_GetGain(void)
{
    return hcs1237.gain;
}

/**
  * @brief  获取当前采样速率
  * @param  无
  * @retval 采样速率
  */
uint16_t Weight_GetSpeed(void)
{
    return hcs1237.speed;
}

/**
  * @brief  获取CS1237句柄
  * @param  无
  * @retval CS1237句柄指针
  */
CS1237_HandleTypeDef* Weight_GetHandle(void)
{
    return &hcs1237;
}

/**
  * @brief  更新重量值并进行滤波
  * @param  无
  * @retval HAL状态
  */
HAL_StatusTypeDef Weight_Update(void)
{
    int32_t raw_data;
    
    if (CS1237_ReadData(&hcs1237, &raw_data) != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    g_currentWeight_0 = Weight_ConvertAdcToWeight(raw_data);
    
    g_weightLimit = g_isLimitExceeded ? 888 : 444;
    int32_t weightDiff = g_currentWeight_0 - g_previousWeight;
    
    if (weightDiff > g_weightLimit)
    {
        g_currentWeight_0 = g_previousWeight;
        g_isLimitExceeded = 1;
    }
    else
    {
        g_isLimitExceeded = 0;
    }
    g_previousWeight = g_currentWeight_0;

    // Kalman filter update
    g_kalmanGain = g_estimateError / (g_measurementError + g_estimateError);
    g_estimatedWeight += g_kalmanGain * (g_currentWeight_0 - g_estimatedWeight);
    g_estimateError = (1 - g_kalmanGain) * g_estimateError + 0.666;
    
    return HAL_OK;
}

/**
  * @brief  ADC值转换为重量值
  * @param  adc_value: ADC值
  * @retval 重量值
  */
int32_t Weight_ConvertAdcToWeight(int32_t adc_value)
{
    return (adc_value - weight_offset) / weight_factor;
}

/**
  * @brief  获取当前滤波后的重量值
  * @param  无
  * @retval 滤波后的重量值
  */
int32_t Weight_GetFilteredWeight(void)
{
    return g_estimatedWeight;
}

/**
  * @brief  获取当前原始重量值
  * @param  无
  * @retval 原始重量值
  */
int32_t Weight_GetCurrentWeight(void)
{
    return g_currentWeight_0;
}

/**
  * @brief  获取重量限制状态
  * @param  无
  * @retval 是否超出限制
  */
uint8_t Weight_IsLimitExceeded(void)
{
    return g_isLimitExceeded;
}

/**
  * @brief  设置重量限制值
  * @param  limit: 限制值
  * @retval HAL状态
  */
HAL_StatusTypeDef Weight_SetLimit(int32_t limit)
{
    if (limit < 0)
    {
        return HAL_ERROR;
    }
    g_weightLimit = limit;
    return HAL_OK;
} 