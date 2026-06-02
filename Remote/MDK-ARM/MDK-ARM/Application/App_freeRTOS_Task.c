#include "App_freeRTOS_Task.h"
#include "FreeRTOS.h"
#include "task.h"

//电源管理任务
void POWER_task(void *pvParameters);
#define POWER_TASK_STACK_SIZE 128
#define POWER_TASK_PRIORITY 1
TaskHandle_t POWER_task_handle;
#define POWER_TASK_DELAY 10000

//通讯任务
void COM_task(void *pvParameters);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 3
TaskHandle_t COM_task_handle;
#define COM_TASK_DELAY 6

//按键任务
void KEY_task(void *pvParameters);
#define KEY_TASK_STACK_SIZE 128
#define KEY_TASK_PRIORITY 2
TaskHandle_t KEY_task_handle;
#define KEY_TASK_DELAY 20

//摇杆任务
void JOYSTICK_task(void *pvParameters);
#define JOYSTICK_TASK_STACK_SIZE 128
#define JOYSTICK_TASK_PRIORITY 2
TaskHandle_t JOYSTICK_task_handle;
#define JOYSTICK_TASK_DELAY 20

/**
 * @brief 启动FreeRTOS操作系统
 */
void App_freeRTOS_Start(void)
{
    //创建电源管理任务
    xTaskCreate(POWER_task, "POWER_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &POWER_task_handle);

    //创建通讯任务
    xTaskCreate(COM_task, "COM_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &COM_task_handle);
    
    //创建按键任务
    xTaskCreate(KEY_task, "KEY_task", KEY_TASK_STACK_SIZE, NULL, KEY_TASK_PRIORITY, &KEY_task_handle);

    //创建摇杆任务
    xTaskCreate(JOYSTICK_task, "JOYSTICK_task", JOYSTICK_TASK_STACK_SIZE, NULL, JOYSTICK_TASK_PRIORITY, &JOYSTICK_task_handle);

    //启动调度器
    vTaskStartScheduler();
}

void POWER_task(void *pvParameters)
{
    //获取当前基准时钟
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        //每10s执行一次，启动电源，避免自动关机
        vTaskDelayUntil(&xLastWakeTime, POWER_TASK_DELAY);
        //启动电源
        Int_IP5305T_start();

    }
}

void COM_task(void *pvParameters)
{
    //获取当前基准时钟
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        //打包发送数据
        App_transmit_data();
        vTaskDelayUntil(&xLastWakeTime, KEY_TASK_DELAY);
    }
}

void KEY_task(void *pvParameters)
{
    //获取当前基准时钟
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        App_process_key_data();
        vTaskDelayUntil(&xLastWakeTime, KEY_TASK_DELAY);
    }
}

void JOYSTICK_task(void *pvParameters)
{
    //获取当前基准时钟
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    //初始化摇杆ADC
    Int_joystick_Init();
    while (1)
    {
        App_process_joystick_data();
        vTaskDelayUntil(&xLastWakeTime, JOYSTICK_TASK_DELAY);
    }   
}
