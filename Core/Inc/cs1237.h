#ifndef __CS1237_H
#define __CS1237_H

#ifdef __cplusplus
extern "C" {
#endif

#include "py32f0xx_hal.h"

/* CS1237 寄存器地址定义 */
#define CS1237_READ_CONFIG      0x56    
#define CS1237_WRITE_CONFIG      0x65

#define CS1237_WAIT_TIME 500

/* CS1237 配置选项 */
typedef enum {
    CS1237_GAIN_1 = 0,    /* 增益 1 */
    CS1237_GAIN_2,        /* 增益 2 */
    CS1237_GAIN_64,       /* 增益 64 */
    CS1237_GAIN_128       /* 增益 128 */
} CS1237_Gain_t;

typedef enum {
    CS1237_SPEED_10HZ = 0,    /* 10Hz */
    CS1237_SPEED_40HZ,        /* 40Hz */
    CS1237_SPEED_640HZ,       /* 640Hz */
    CS1237_SPEED_1280HZ       /* 1280Hz */
} CS1237_Speed_t;

/* CS1237 结构体定义 */
typedef struct {
    GPIO_TypeDef *sck_port;       /* SCK 端口 */
    uint16_t sck_pin;             /* SCK 引脚 */
    GPIO_TypeDef *dout_port;      /* DOUT 端口 */
    uint16_t dout_pin;            /* DOUT 引脚 */
    CS1237_Gain_t gain;           /* 当前增益设置 */
    CS1237_Speed_t speed;         /* 当前采样速率 */
    uint32_t offset;              /* 零点偏移值 */
    uint32_t gain_factor;         /* 增益系数 */
    uint8_t is_initialized;       /* 初始化标志 */
} CS1237_HandleTypeDef;

/* 函数声明 */
HAL_StatusTypeDef CS1237_Init(CS1237_HandleTypeDef *hcs1237);
HAL_StatusTypeDef CS1237_UpdateConfig(CS1237_HandleTypeDef *hcs1237);
HAL_StatusTypeDef CS1237_ReadData(CS1237_HandleTypeDef *hcs1237, int32_t *data);
HAL_StatusTypeDef CS1237_Calibrate(CS1237_HandleTypeDef *hcs1237);
HAL_StatusTypeDef CS1237_WriteReg(CS1237_HandleTypeDef *hcs1237, uint32_t data);
HAL_StatusTypeDef CS1237_ReadReg(CS1237_HandleTypeDef *hcs1237, uint32_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __CS1237_H */ 