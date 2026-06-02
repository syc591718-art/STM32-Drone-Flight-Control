#ifndef _INT_MPU6050_H_
#define _INT_MPU6050_H_

#include "i2c.h"
#include "Com_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"

//从设备地址
#define MPU6050_ADDR 0x68
//读写地址
#define MPU6050_ADDR_WRITE 0xD0
#define MPU6050_ADDR_READ 0xD1

//初始化MPU6050芯片
void Int_MPU6050_Init(void);

//读取三轴角速度
void Int_MPU6050_Get_Gyro(Gyro_Struct *gyro);

//读取三轴加速度
void Int_MPU6050_Get_Acc(Accel_Struct *acc);

//读取六轴数据
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data);

#endif
