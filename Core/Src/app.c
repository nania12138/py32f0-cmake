#include "app.h"
#include "weight.h"

// Constants
const int32_t THRESHOLDS[] = { 50, 75 };
const int32_t THRESHOLD_COUNT = sizeof(THRESHOLDS) / sizeof(THRESHOLDS[0]);

// Threshold and control variables
int32_t g_threshold;
bool g_isEmaFilterEnabled = true;

void APP_Init(void)
{
    // 初始化CS1237
    Weight_Init(&hcs1237);
    
    // 初始化阈值
    g_threshold = THRESHOLDS[0];
}

void APP_Process(void)
{
    UpdateWeight();
    isProbeLev();
    isProbeGan();
    checkPressureChange();
}

void UpdateWeight(void)
{
    Weight_Update();
}

void checkPressureChange(void)
{
    int32_t filteredWeight = Weight_GetFilteredWeight();
    
    if (filteredWeight > g_threshold)
    {
        HAL_GPIO_WritePin(PROBE_OUT_PORT, PROBE_OUT_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    }
    else if (0 - filteredWeight > g_threshold)
    {
        HAL_GPIO_WritePin(PROBE_OUT_PORT, PROBE_OUT_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(PROBE_OUT_PORT, PROBE_OUT_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    }
}

void isProbeLev(void)
{
    static uint32_t levPressTime = 0;
    static uint8_t isRST = 0;
    
    if (HAL_GPIO_ReadPin(PROBE_LEV_PORT, PROBE_LEV_PIN) == GPIO_PIN_RESET)
    {
        if (isRST == 0)
        {
            levPressTime = HAL_GetTick();
            isRST = 1;
        }
        else if (isRST == 1)
        {
            if (HAL_GetTick() - levPressTime > 50)
            {
                isRST = 2;
                RST0();
            }
        }
    }
    else
    {
        if (isRST != 0)
        {
            isRST = 0;
            levPressTime = 0;
        }
    }
}

void isProbeGan(void)
{
    static uint32_t ganPressTime = 0;
    static uint8_t isGAN = 0;
    
    if (HAL_GPIO_ReadPin(PROBE_GAN_PORT, PROBE_GAN_PIN) == GPIO_PIN_RESET)
    {
        if (isGAN == 0)
        {
            ganPressTime = HAL_GetTick();
            isGAN = 1;
        }
        else if (isGAN == 1)
        {
            if (HAL_GetTick() - ganPressTime > 1000)
            {
                isGAN = 2;
                if (HAL_GPIO_ReadPin(PROBE_LEV_PORT, PROBE_LEV_PIN) == GPIO_PIN_RESET)
                {
                    g_threshold = THRESHOLDS[0];
                }
                else
                {
                    GAN();
                }
            }
        }
    }
    else
    {
        if (isGAN != 0)
        {
            isGAN = 0;
            ganPressTime = 0;
        }
    }
}

void RST0(void)
{
    Weight_Calibrate();
}

void GAN(void)
{
    Weight_Calibrate();
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
} 