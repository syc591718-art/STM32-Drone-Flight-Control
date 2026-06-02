#ifndef __INT_MOTOR_H__
#define __INT_MOTOR_H__

#include "tim.h"
#include "Com_debug.h"

typedef struct 
{
    TIM_HandleTypeDef *tim;
    uint16_t channel;
    int16_t speed;        
}Motor_Struct;

//设置马达转速
//传入的参数是占空比相对的值，最大1000，默认200
void Int_motor_set_speed(Motor_Struct *motor);

//启动电机
void Int_motor_start(Motor_Struct *motor);

#endif
