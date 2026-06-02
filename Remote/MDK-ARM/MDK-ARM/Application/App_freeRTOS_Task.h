#ifndef __APP_FREERTOS_TASK_H__
#define __APP_FREERTOS_TASK_H__


#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Int_IP5305T.h"
#include "Int_SI24R1.h"
#include "App_process_data.h"
#include "App_transmit_data.h"

/**
 * @brief 启动FreeRTOS操作系统
 */
void App_freeRTOS_Start(void);
void task1(void *pvParameters);
void task2(void *pvParameters);

#endif 
