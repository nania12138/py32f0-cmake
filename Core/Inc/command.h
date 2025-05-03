#ifndef __COMMAND_H
#define __COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cs1237.h"
#include "weight.h"

/* 函数声明 */
void Command_Init(void);
void Command_Process(void);
void Command_StartContinuousReading(void);
void Command_StopContinuousReading(void);

extern  CS1237_HandleTypeDef hcs1237;

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_H */ 