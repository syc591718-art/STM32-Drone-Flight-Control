#ifndef _INT_VL53L1X_H_
#define _INT_VL53L1X_H_

#include "vl53l1_platform.h"
#include "VL53L1X_api.h"
#include "VL53L1X_calibration.h"
#include "FreeRTOS.h"
#include "task.h"

//初始化激光测距仪，完成寄存器配置
void Int_VL53L1X_Init(void);

//读取激光测距仪的距离值
uint16_t Int_VL53L1X_Get_Distance(void);

#endif 
