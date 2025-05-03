#ifndef __APP_H
#define __APP_H

#include "py32f0xx_hal.h"
#include "CS1237.h"
#include <stdbool.h>
#include <stdlib.h>

// Pin definitions
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

// Constants

// Function declarations
void APP_Init(void);
void APP_Process(void);
void UpdateWeight(void);
int32_t adc_to_weight(uint32_t pin_sck, int32_t adc_value);
void RST0(void);
void GAN(void);
int32_t GetOriginMid(uint32_t pin_sck, uint32_t pin_dout);
void checkPressureChange(void);
void isProbeLev(void);
void isProbeGan(void);

// External variables
extern UART_HandleTypeDef huart1;

#endif /* __APP_H */ 