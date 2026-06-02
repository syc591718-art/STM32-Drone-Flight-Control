#ifndef __APP_FLIGHT_H__
#define __APP_FLIGHT_H__

#include "math.h"
#include "Com_debug.h"
#include "Com_filter.h"
#include "Com_imu.h"
#include "Com_pid.h"
#include "Int_motor.h"
#include "Int_mpu6050.h"
#include "Int_VL53L1X.h"

//飞控任务初始化：MPU6050初始化、电机初始化
void App_flight_init(void);

//根据陀螺仪测量的数据计算欧拉角
void App_flight_get_euler_angle(void);

//根据欧拉角计算出PID的目标值
void App_flight_pid_process(void);

//根据PID的输出值控制电机
void App_flight_control_motor(void);

//定高功能中PID计算
void App_flight_fix_height_pid_process(void);

#endif
