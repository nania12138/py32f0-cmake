#include "cs1237.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"

/* 私有函数声明 */
static void Command_PrintHelp(void);
static void Command_ProcessCommand(char *cmd);
static void Command_PrintData(int32_t data, uint8_t gain, uint8_t speed);
static char* Command_TrimLeft(char *str, size_t trim_len);
static uint8_t Command_GetGainValue(char *str);
static uint16_t Command_GetSpeedValue(char *str);

/* 命令缓冲区 */
#define CMD_BUFFER_SIZE 32
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

/* 命令处理函数 */
void Command_Init(void)
{
    /* 初始化串口接收缓冲区 */
    cmd_index = 0;
    memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
    
    /* 配置串口 */
    USART_Config();
    
    /* 打印欢迎信息 */
    printf("\r\nCS1237 ADC Control System\r\n");
    printf("Type 'help' for available commands\r\n");
}

void Command_Process(void)
{
    /* 检查是否有新数据 */
    if (HAL_UART_Receive(USART_GetHandle(), (uint8_t *)&cmd_buffer[cmd_index], 1, 0) == HAL_OK)
    {
        /* 处理回车键 */
        if (cmd_buffer[cmd_index] == '\r' || cmd_buffer[cmd_index] == '\n')
        {
            if (cmd_index > 0)
            {
                cmd_buffer[cmd_index] = '\0';
                Command_ProcessCommand(cmd_buffer);
                cmd_index = 0;
                memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
            }
        }
        /* 处理退格键 */
        else if (cmd_buffer[cmd_index] == '\b' || cmd_buffer[cmd_index] == 0x7F)
        {
            if (cmd_index > 0)
            {
                cmd_index--;
                printf("\b \b");
            }
        }
        /* 正常字符 */
        else if (cmd_index < CMD_BUFFER_SIZE - 1)
        {
            printf("%c", cmd_buffer[cmd_index]);
            cmd_index++;
        }
    }
}

/* 从字符串开头去除指定数量的字符 */
static char* Command_TrimLeft(char *str, size_t trim_len)
{
    if (str == NULL) {
        return NULL;
    }
    
    size_t str_len = strlen(str);
    if (trim_len >= str_len) {
        return str + str_len;  // Return pointer to end of string
    }
    
    return str + trim_len;
}

/* 获取增益值 */
static uint8_t Command_GetGainValue(char *str)
{
    if (str == NULL) return 0;
    char *value = Command_TrimLeft(str, 5);
    return (uint8_t)atoi(value);
}

/* 获取速度值 */
static uint16_t Command_GetSpeedValue(char *str)
{
    if (str == NULL) return 0;
    char *value = Command_TrimLeft(str, 6);
    return (uint16_t)atoi(value);
}

/* 打印帮助信息 */
static void Command_PrintHelp(void)
{
    printf("\r\nAvailable commands:\r\n");
    printf("help    - Show this help message\r\n");
    printf("read    - Read ADC data\r\n");
    printf("gain    - Set gain (1,2,64,128)\r\n");
    printf("speed   - Set speed (10,40,640,1280)\r\n");
    printf("cal     - Calibrate ADC\r\n");
    printf("start   - Start continuous reading\r\n");
    printf("stop    - Stop continuous reading\r\n");
}

/* 处理命令 */
static void Command_ProcessCommand(char *cmd)
{
    printf("\r\n");
    
    if (strcmp(cmd, "help") == 0)
    {
        Command_PrintHelp();
    }
    else if (strcmp(cmd, "read") == 0)
    {
        int32_t data;
        if (CS1237_ReadData(&hcs1237, &data) == HAL_OK)
        {
            Command_PrintData(data, hcs1237.gain, hcs1237.speed);
        }
        else
        {
            printf("Error reading data\r\n");
        }
    }
    else if (strncmp(cmd, "gain", 4) == 0)
    {
        uint8_t gain = Command_GetGainValue(cmd);
        CS1237_Gain_t gain_setting;
        
        switch(gain)
        {
            case 1:   gain_setting = CS1237_GAIN_1; break;
            case 2:   gain_setting = CS1237_GAIN_2; break;
            case 64:  gain_setting = CS1237_GAIN_64; break;
            case 128: gain_setting = CS1237_GAIN_128; break;
            default:
                printf("Invalid gain value. Use: 1, 2, 64, or 128\r\n");
                return;
        }
        
        hcs1237.gain = gain_setting;
        if (CS1237_UpdateConfig(&hcs1237) == HAL_OK)
        {
            printf("Gain set to %d\r\n", gain);
        }
        else
        {
            printf("Error setting gain\r\n");
        }
    }
    else if (strncmp(cmd, "speed", 5) == 0)
    {
        uint16_t speed = Command_GetSpeedValue(cmd);
        CS1237_Speed_t speed_setting;
        
        switch(speed)
        {
            case 10:   speed_setting = CS1237_SPEED_10HZ; break;
            case 40:   speed_setting = CS1237_SPEED_40HZ; break;
            case 640:  speed_setting = CS1237_SPEED_640HZ; break;
            case 1280: speed_setting = CS1237_SPEED_1280HZ; break;
            default:
                printf("Invalid speed value. Use: 10, 40, 640, or 1280\r\n");
                return;
        }
        
        hcs1237.speed = speed_setting;
        if (CS1237_UpdateConfig(&hcs1237) == HAL_OK)
        {
            printf("Speed set to %d Hz\r\n", speed);
        }
        else
        {
            printf("Error setting speed\r\n");
        }
    }
    else if (strcmp(cmd, "cal") == 0)
    {
        printf("Calibrating...\r\n");
        if (CS1237_Calibrate(&hcs1237) == HAL_OK)
        {
            printf("Calibration complete\r\n");
        }
        else
        {
            printf("Calibration failed\r\n");
        }
    }
    else
    {
        printf("Unknown command. Type 'help' for available commands\r\n");
    }
    
    printf("> ");
}

/* 打印数据 */
static void Command_PrintData(int32_t data, uint8_t gain, uint8_t speed)
{
    printf("ADC Data: %ld\r\n", data);
    printf("Gain: %d\r\n", gain);
    printf("Speed: %d Hz\r\n", speed);
}

/* 连续读取标志 */
static uint8_t continuous_reading = 0;

/* 开始连续读取 */
void Command_StartContinuousReading(void)
{
    continuous_reading = 1;
    printf("Starting continuous reading...\r\n");
    printf("Press any key to stop\r\n");
    
    while(continuous_reading)
    {
        int32_t data;
        if (CS1237_ReadData(&hcs1237, &data) == HAL_OK)
        {
            Command_PrintData(data, hcs1237.gain, hcs1237.speed);
        }
        
        /* 检查是否有按键输入 */
        if (HAL_UART_Receive(USART_GetHandle(), (uint8_t *)cmd_buffer, 1, 0) == HAL_OK)
        {
            continuous_reading = 0;
            printf("\r\nStopped continuous reading\r\n");
        }
        
        /* 根据采样速率延时 */
        switch(hcs1237.speed)
        {
            case CS1237_SPEED_10HZ:
                HAL_Delay(100);
                break;
            case CS1237_SPEED_40HZ:
                HAL_Delay(25);
                break;
            case CS1237_SPEED_640HZ:
                HAL_Delay(2);
                break;
            case CS1237_SPEED_1280HZ:
                HAL_Delay(1);
                break;
        }
    }
}

/* 停止连续读取 */
void Command_StopContinuousReading(void)
{
    continuous_reading = 0;
} 