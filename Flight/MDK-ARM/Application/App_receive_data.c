#include "App_receive_data.h"

//存放数据结构体
extern Remote_Data_Struct remote_data;

uint8_t receive_buff[TX_PLOAD_WIDTH] = {0};

extern Remote_State remote_state;

extern Flight_State flight_state;

extern uint16_t fix_height;

Thr_State thr_state = FREE;

//MAX状态进入时间
uint32_t max_enter_time = 0;

//MIN状态进入时间
uint32_t min_enter_time = 0;

//重试次数

uint8_t retry_count = 0;

//接受遥控器发送的数据
//0表示校验成功，接受正常；1表示失败
uint8_t App_receive_data(void)
{
    memset(receive_buff, 0, TX_PLOAD_WIDTH);
    Int_SI24R1_RxPacket(receive_buff);

    if(strlen((char*)receive_buff) == 0)
    {
        return 1;
    }

    //帧头帧尾校验
    if(receive_buff[0] != FRAME_HEAD_CHECK || receive_buff[1]!= FRAME_HEAD_CHECK)
    {
        return 1;
    }
    if(receive_buff[16] != FRAME_TAIL_CHECK || receive_buff[17] != FRAME_TAIL_CHECK)
    {
        return 1;
    }
    //和校验
    uint32_t sum = 0;
    uint32_t sum_receive = 0;
    for(uint8_t i = 0; i < 12; i++)
    {
        sum += receive_buff[i];
    }
    sum_receive = receive_buff[12] << 24 |receive_buff[13] << 16 | receive_buff[14] << 8 | receive_buff[15];
    if(sum != sum_receive)
    {
        return 1;
    }

    //保存数据
    remote_data.thr = receive_buff[2] << 8 | receive_buff[3];
    remote_data.yaw = receive_buff[4] << 8 | receive_buff[5];
    remote_data.pit = receive_buff[6] << 8 | receive_buff[7];
    remote_data.rol = receive_buff[8] << 8 | receive_buff[9];
    remote_data.shutdown = receive_buff[10];
    remote_data.fix_height = receive_buff[11];

    return 0;
}

//处理连接状态
void App_process_connect_state(uint8_t res)
{
    if(res == 0)
    {
        remote_state = REMOTE_CONNECTED;
        retry_count = 0;
    }
    else if(res == 1)
    {
        retry_count++;
        if(retry_count == MAX_RETRY_TIMES)
        {
            remote_state = REMOTE_DISCONNECTED;
            retry_count = 0;
        }
    }
}

//处理解锁逻辑，返回0表示解锁成功，1表示解锁失败
static uint8_t App_process_unlock(void)
{
    //为保证安全，解锁初始状态油门为0
    switch (thr_state)
    {
    case FREE:
        if(remote_data.thr > 900)
        {
            thr_state = MAX;
            max_enter_time = xTaskGetTickCount();
        }
        break;
    case MAX:
        if(remote_data.thr < 900)
        {
            if(xTaskGetTickCount() - max_enter_time > 1000)
            thr_state = LEAVE_MAX;
            else
            {
                thr_state = FREE;
            }
        }
        break;
    case LEAVE_MAX:
        if(remote_data.thr <= 100)
        {
            thr_state = MIN;
            min_enter_time = xTaskGetTickCount();
        }
        break;
    case MIN:
        if(xTaskGetTickCount() - min_enter_time <= 1000)
        {
            if(remote_data.thr > 100)
            {
                thr_state = FREE;
            }
        }
        else
        {
            thr_state = UNLOCK;
        }
        break;
    case UNLOCK:
        break;
    default:
        break;
    }
    if(thr_state == UNLOCK)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

//处理飞行状态
void App_process_flight_state(void)
{
    static uint32_t lost_start_time = 0;
    static uint8_t last_disconnected = 0;
    //使用状态机逻辑实现
    switch(flight_state)
    {
        case IDLE:
            if(App_process_unlock() == 0)
            {
                flight_state = NORMAL;
                thr_state = FREE;
            }
            break;

        case NORMAL:
            if(remote_data.fix_height == 1)
            {
                flight_state = FIX_HEIGHT;
                remote_data.fix_height = 0;

                //记录当前目标高度
                fix_height = Int_VL53L1X_Get_Distance();
            }
            if(remote_state == REMOTE_DISCONNECTED)
            {
                if(!last_disconnected)
                {
                    lost_start_time = xTaskGetTickCount();
                    last_disconnected = 1;
                }
                else
                {
                    // 失联时间过长，进入FAIL状态
                    if(xTaskGetTickCount() - lost_start_time > 500)
                    {
                        flight_state = FAIL;
                        last_disconnected = 0;
                    }
                }
            }
            else
            {
                last_disconnected = 0;
            }
            break;

        case FIX_HEIGHT:
            if(remote_data.fix_height == 1)
            {
                flight_state = NORMAL;
                remote_data.fix_height = 0;
            }
            if(remote_state == REMOTE_DISCONNECTED)
            {
                flight_state = FAIL;
            }
            break;

        case FAIL:
            //处理失联故障，缓慢停止电机
            //等待故障处理完成
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            flight_state = IDLE;
            break;
            
        default:
            break;
    }
}
