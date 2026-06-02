#ifndef __COM_CONFIG_H__
#define __COM_CONFIG_H__

#include "main.h"

// 遥控器连接状态
typedef enum
{
    REMOTE_CONNECTED = 0,
    REMOTE_DISCONNECTED = 1,
}Remote_State;

//飞行状态
typedef enum
{
    IDLE = 0,
    NORMAL = 1,
    FIX_HEIGHT = 2,
    FAIL = 3,
}Flight_State;

//油门解锁状态
typedef enum
{
    FREE = 0,
    MAX,
    LEAVE_MAX,
    MIN,
    UNLOCK,
}Thr_State;

//数据帧数据内容结构体
typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t rol;
    int16_t pit;
    uint8_t shutdown;   // 0:不关机 1:关机
    uint8_t fix_height; // 0:不切换 1:切换定高或不定高
} Remote_Data_Struct;

//陀螺仪加速度数据
typedef struct
{
    int16_t accel_x;//往前的加速度为正
    int16_t accel_y;//往左的加速度为正
    int16_t accel_z;//朝上的加速度为正
}Accel_Struct;

//陀螺仪角速度数据
typedef struct
{
    int16_t gyro_x;//向右飞转动为正，表示横滚角
    int16_t gyro_y;//向前飞转动为正，表示俯仰角
    int16_t gyro_z;//逆时针转动为正，表示偏航角
}Gyro_Struct;

//陀螺仪结构体
typedef struct
{
    Gyro_Struct gyro;
    Accel_Struct accel;
}Gyro_Accel_Struct;

//解算得到的欧拉角
typedef struct 
{
    float yaw;
    float rol;
    float pit;
}Euler_Struct;


#endif
