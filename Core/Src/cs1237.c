#include "cs1237.h"

/* 私有函数声明 */
static void CS1237_Delay();
static void CS1237_SCK_Low(CS1237_HandleTypeDef *hcs1237);
static void CS1237_SCK_High(CS1237_HandleTypeDef *hcs1237);
static void CS1237_DOUT_Low(CS1237_HandleTypeDef *hcs1237);
static void CS1237_DOUT_High(CS1237_HandleTypeDef *hcs1237);
static uint8_t CS1237_DOUT_Read(CS1237_HandleTypeDef *hcs1237);
static void CS1237_WriteBit(CS1237_HandleTypeDef *hcs1237, uint8_t data);
static uint8_t CS1237_ReadBit(CS1237_HandleTypeDef *hcs1237);

/**
  * @brief  更新 CS1237 配置
  * @param  hcs1237: CS1237 句柄
  * @retval HAL 状态
  */
HAL_StatusTypeDef CS1237_UpdateConfig(CS1237_HandleTypeDef *hcs1237)
{
    if (hcs1237 == NULL || !hcs1237->is_initialized)
    {
        return HAL_ERROR;
    }

    uint32_t reg_value = 0;
    if (CS1237_ReadReg(hcs1237, &reg_value) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* 清除旧的配置并设置新的配置 *///TODO:核对寄存器位置
    reg_value &= ~(0x0F);  // 清除增益和速率位
    reg_value |= ((hcs1237->gain & 0x03) << 2) | (hcs1237->speed & 0x03);  // 设置新的增益和速率

    if (CS1237_WriteReg(hcs1237, reg_value) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
  * @brief  初始化 CS1237
  * @param  hcs1237: CS1237 句柄
  * @param  sck_port: SCK 端口
  * @param  sck_pin: SCK 引脚
  * @param  dout_port: DOUT 端口
  * @param  dout_pin: DOUT 引脚
  * @retval HAL 状态
  */
HAL_StatusTypeDef CS1237_Init(CS1237_HandleTypeDef *hcs1237)
{
    if (hcs1237 == NULL)
    {
        return HAL_ERROR;
    }

    /* 初始化结构体成员 */ //TODO：增加到主函数中
    hcs1237->sck_port = GPIOA;
    hcs1237->sck_pin = GPIO_PIN_5;
    hcs1237->dout_port = GPIOA;
    hcs1237->dout_pin = GPIO_PIN_7;
    hcs1237->gain = CS1237_GAIN_128;
    hcs1237->speed = CS1237_SPEED_1280HZ;
    hcs1237->offset = 0;
    hcs1237->gain_factor = 1;
    hcs1237->is_initialized = 0;

    /* 配置 SCK 引脚为输出 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hcs1237->sck_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//TODO:核对是否要上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hcs1237->sck_port, &GPIO_InitStruct);

    /* 配置 DOUT 引脚为输入 */
    GPIO_InitStruct.Pin = hcs1237->dout_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(hcs1237->dout_port, &GPIO_InitStruct);

    /* 设置默认配置 */
    hcs1237->is_initialized = 1;  // 临时设置初始化标志以允许配置更新
    if (CS1237_UpdateConfig(hcs1237) != HAL_OK)
    {
        hcs1237->is_initialized = 0;
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
  * @brief  读取 ADC 数据
  * @param  hcs1237: CS1237 句柄
  * @param  data: 数据指针
  * @retval HAL 状态
  */
HAL_StatusTypeDef CS1237_ReadData(CS1237_HandleTypeDef *hcs1237, int32_t *data)
{
    if (hcs1237 == NULL || data == NULL || !hcs1237->is_initialized)
    {
        return HAL_ERROR;
    }
    /* 等待 DOUT 变高 */
    if(CS1237_DOUT_Read(hcs1237) == 0)
    {
        return HAL_BUSY;
    }

    uint32_t raw_data = 0;
    for(int i = 0; i < 24; i++)
    {
        raw_data = (raw_data << 1) | CS1237_ReadBit(hcs1237);
    }

    /* 转换为有符号数据 */
    if (raw_data & 0x800000)
    {
        raw_data |= 0xFF000000;
    }

    return HAL_OK;
}

/**
  * @brief  校准 ADC
  * @param  hcs1237: CS1237 句柄
  * @retval HAL 状态
  */
HAL_StatusTypeDef CS1237_Calibrate(CS1237_HandleTypeDef *hcs1237)
{
    if (hcs1237 == NULL || !hcs1237->is_initialized)
    {
        return HAL_ERROR;
    }

    /* 读取多次数据取平均值作为零点偏移 */
    int32_t sum = 0;
    const int samples = 10;
    
    for (int i = 0; i < samples; i++)
    {
        int32_t data;
        if (CS1237_ReadData(hcs1237, &data) != HAL_OK)
        {
            return HAL_ERROR;
        }
        sum += data;
        CS1237_Delay(100);
    }

    hcs1237->offset = sum / samples;
    return HAL_OK;
}

/**
  * @brief  写寄存器
  * @param  hcs1237: CS1237 句柄
  * @param  reg: 寄存器地址
  * @param  data: 要写入的数据
  * @retval HAL 状态
  */
HAL_StatusTypeDef CS1237_WriteReg(CS1237_HandleTypeDef *hcs1237, uint32_t data)
{
    if (hcs1237 == NULL || !hcs1237->is_initialized)
    {
        return HAL_ERROR;
    }

    uint32_t temp_data = 0;
    while(CS1237_ReadData(hcs1237, &temp_data) != HAL_OK)//TODO：决定读取前寄存器的数据是否保留
    {
        // return HAL_ERROR;
    }

    uint8_t bit25 = CS1237_ReadBit(hcs1237);
    uint8_t bit26 = CS1237_ReadBit(hcs1237);

    for(int i = 0; i < 8; i++)
    {
        uint8_t bit = data & 0x80;
        CS1237_WriteBit(hcs1237, bit);
        data <<= 1;
    }
    return HAL_OK;
}

/**
  * @brief  读寄存器
  * @param  hcs1237: CS1237 句柄
  * @param  reg: 寄存器地址
  * @param  data: 数据指针
  * @retval HAL 状态
  */
HAL_StatusTypeDef CS1237_ReadReg(CS1237_HandleTypeDef *hcs1237, uint32_t *data)
{
    if (hcs1237 == NULL || data == NULL || !hcs1237->is_initialized)
    {
        return HAL_ERROR;
    }

    uint32_t temp_data = 0;
    while(CS1237_ReadData(hcs1237, &temp_data) != HAL_OK)//TODO：决定读取前寄存器的数据是否保留
    {
        // return HAL_ERROR;
    }

    uint8_t bit25 = CS1237_ReadBit(hcs1237);
    uint8_t bit26 = CS1237_ReadBit(hcs1237);

    /* 读取config寄存器 */
    for(int i = 0; i < 8; i++)
    {
        uint8_t bit = CS1237_ReadBit(hcs1237);
        *data = (*data << 1) | bit;
    }

    return HAL_OK;
}

/* 私有函数实现 */
static void CS1237_SCK_Low(CS1237_HandleTypeDef *hcs1237)
{
    HAL_GPIO_WritePin(hcs1237->sck_port, hcs1237->sck_pin, GPIO_PIN_RESET);
}

static void CS1237_SCK_High(CS1237_HandleTypeDef *hcs1237)
{
    HAL_GPIO_WritePin(hcs1237->sck_port, hcs1237->sck_pin, GPIO_PIN_SET);
}

static void CS1237_DOUT_Low(CS1237_HandleTypeDef *hcs1237)
{
    HAL_GPIO_WritePin(hcs1237->dout_port, hcs1237->dout_pin, GPIO_PIN_RESET);
}

static void CS1237_DOUT_High(CS1237_HandleTypeDef *hcs1237)
{
    HAL_GPIO_WritePin(hcs1237->dout_port, hcs1237->dout_pin, GPIO_PIN_SET);
}

static uint8_t CS1237_DOUT_Read(CS1237_HandleTypeDef *hcs1237)
{
    return HAL_GPIO_ReadPin(hcs1237->dout_port, hcs1237->dout_pin) == GPIO_PIN_SET ? 1 : 0;
}

static void CS1237_WriteBit(CS1237_HandleTypeDef *hcs1237, uint8_t data)
{
    CS1237_SCK_High(hcs1237);
    if(data == 1)
    {
        CS1237_DOUT_High(hcs1237);
    }
    else
    {
        CS1237_DOUT_Low(hcs1237);
    }
    CS1237_Delay(); 
    CS1237_SCK_Low(hcs1237);
    CS1237_Delay(); 
}

static uint8_t CS1237_ReadBit(CS1237_HandleTypeDef *hcs1237)
{
    CS1237_SCK_High(hcs1237);
    CS1237_Delay(); 
    uint8_t data = CS1237_DOUT_Read(hcs1237);
    CS1237_SCK_Low(hcs1237);
    CS1237_Delay(); 
    return data;
}

static void CS1237_Delay()
{
    /* 简单的循环延时，根据系统时钟频率调整循环次数 */
    volatile uint32_t count = (CS1237_WAIT_TIME * (SystemCoreClock / 1000000000)) / 4;
    while(count--);
}
