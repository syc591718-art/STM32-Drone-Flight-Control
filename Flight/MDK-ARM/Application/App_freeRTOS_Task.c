#include "App_freeRTOS_Task.h"

//固定结构体，循环使用
//LED结构体
LED_Struct left_top_led = {.port = LED1_GPIO_Port, .pin = LED1_Pin};
LED_Struct right_top_led = {.port = LED2_GPIO_Port, .pin = LED2_Pin};
LED_Struct right_bottom_led= {.port = LED3_GPIO_Port, .pin = LED3_Pin};
LED_Struct left_bottom_led= {.port = LED4_GPIO_Port, .pin = LED4_Pin};

//表示当前连接状态
Remote_State remote_state = REMOTE_DISCONNECTED;

//表示当前飞行状态
Flight_State flight_state = IDLE;

//遥控器接收数据
Remote_Data_Struct remote_data = {.thr = 0,.yaw = 500,.pit = 500,.rol = 500,.shutdown = 0,.fix_height = 0};

//定高时的飞行高度
uint16_t fix_height = 0;

//电源管理任务
void POWER_task(void *pvParameters);
#define POWER_TASK_STACK_SIZE 128
#define POWER_TASK_PRIORITY 4
#define POWER_TASK_DELAY 10000
TaskHandle_t POWER_task_handle;

//飞行控制任务
void FLIGHT_task(void *pvParameters);
#define FLIGHT_TASK_STACK_SIZE 128
#define FLIGHT_TASK_PRIORITY 3
#define FLIGHT_TASK_DELAY 6
TaskHandle_t FLIGHT_task_handle;

//LED任务
void LED_task(void *pvParameters);
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1
#define LED_TASK_DELAY 100
TaskHandle_t LED_task_handle;

//通讯任务
void COM_task(void *pvParameters);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 4
#define COM_TASK_DELAY 10
TaskHandle_t COM_task_handle;

//启动FreeRTOS操作系统
void App_freeRTOS_Start(void)
{
    //创建电源管理任务
    xTaskCreate(POWER_task, "POWER_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &POWER_task_handle);

    //创建飞行控制任务
    xTaskCreate(FLIGHT_task, "FLIGHT_task", FLIGHT_TASK_STACK_SIZE, NULL, FLIGHT_TASK_PRIORITY, &FLIGHT_task_handle);

    //创建LED任务
    xTaskCreate(LED_task, "LED_task", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, &LED_task_handle);

    //创建通讯任务
    xTaskCreate(COM_task, "COM_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &COM_task_handle);

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
        uint32_t res = ulTaskNotifyTake(pdTRUE,POWER_TASK_DELAY);
        if(res != 0)
        {
            //收到通知，关机
            Int_IP5305T_shutdown();
        }
        else
        {
            //没有收到通知，正常启动
            Int_IP5305T_start();
        }
        vTaskDelayUntil(&xLastWakeTime, FLIGHT_TASK_DELAY);
    }

}

void FLIGHT_task(void *pvParameters)
{
    //获取当前基准时钟
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;
    App_flight_init();
    while (1)
    {
        //根据MPU6050测量的数据，姿态解算得到欧拉角
        App_flight_get_euler_angle();

        //根据当前欧拉角进行PID解算控制
        App_flight_pid_process();

        //判断定高
        if(flight_state == FIX_HEIGHT)
        {
            //激光测距仪的数据采集 20ms一次
            count ++;
            if(count >= 4)
            {
                App_flight_fix_height_pid_process();
                count = 0;
            }
        }

        //根据PID计算的结果，对电机进行控制
        App_flight_control_motor();

        vTaskDelayUntil(&xLastWakeTime, FLIGHT_TASK_DELAY);
    }
}

void LED_task(void *pvParameters)
{
    //获取当前基准时钟
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;
    while (1)
    {
        count++;
        //判断当前连接状态,前两个灯表示连接状态
        if(remote_state == REMOTE_CONNECTED)
        {
            //点亮前两个灯
            Int_led_turn_on(&left_top_led);
            Int_led_turn_on(&right_top_led);
        }
        else if(remote_state == REMOTE_DISCONNECTED)
        {
            //熄灭前两个灯
            Int_led_turn_off(&left_top_led);
            Int_led_turn_off(&right_top_led);
        }

        //判断飞行状态，后两个等表示飞行状态
        if(flight_state == IDLE)//空闲状态
        {
            //灯慢闪，500ms闪
            if(count % 5 == 0)
            {
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
        }
        else if (flight_state == NORMAL)//常规状态
        {
            //灯快闪，200ms闪
            if(count % 2 == 0)
            {
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
        }
        else if (flight_state == FIX_HEIGHT)//定高状态
        {
            //点亮后两个灯
            Int_led_turn_on(&left_bottom_led);
            Int_led_turn_on(&right_bottom_led);
        }
        else if(flight_state == FAIL)//故障状态
        {
            //熄灭后两个灯
            Int_led_turn_off(&left_bottom_led);
            Int_led_turn_off(&right_bottom_led);
        }

        //重置count计数
        if(count == 10)
        {
            count = 0;
        }
        
        vTaskDelayUntil(&xLastWakeTime, LED_TASK_DELAY);
    }
}

void COM_task(void *pvParameters)
{
    //获取当前基准时钟
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        //接收数据
        uint8_t res = App_receive_data();
        
        //处理连接状态
        App_process_connect_state(res);

        //处理关机命令
        if(remote_data.shutdown == 1)
        {
            //使用直接任务通知 => 通知电源任务关机
            xTaskNotifyGive(POWER_task_handle);
        }

        //处理飞机飞行状态
        App_process_flight_state();

        vTaskDelay(COM_TASK_DELAY);
    }
}
