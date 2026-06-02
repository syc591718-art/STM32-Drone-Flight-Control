#ifndef __COM_PID_H__
#define __COM_PID_H__

#define PID_PERIOD 0.006
#include "main.h"

//PID结构体
//kp,ki,kd需要在初始化时确定
typedef struct 
{
    float kp;//比例：响应速度
    float ki;//积分：解决稳态误差（无人机部分不使用）
    float kd;//微分：值越大，抑制效果越好，解决过调震荡
    float err;//误差值
    float desire;//目标值
    float measure;//测量值
    float last_err;//上一次误差值
    float integral;//积分累计
    float output;//输出结果
}PID_Struct;

//单词PID计算
void Com_PID_Calc(PID_Struct *pid);

//串级PID计算
void Com_PID_Calc_Chain(PID_Struct *out_pid, PID_Struct *in_pid);

//限制数值在正常范围内
int16_t Com_limit(int16_t speed,int16_t max_speed,int16_t min_speed);

#endif
