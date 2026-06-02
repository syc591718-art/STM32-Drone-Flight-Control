#include "Com_pid.h"

//单词PID计算
void Com_PID_Calc(PID_Struct *pid)
{
    //计算误差值 = 测量值 - 目标值
    pid->err = pid->measure - pid->desire;

    //计算积分误差
    pid->integral += pid->err;

    if(pid->last_err == 0)
    {
        pid->last_err = pid->err;
    }
    
    //计算微分误差
    float der = pid->err - pid->last_err;

    //计算输出
    pid->output = (pid->kp * pid->err) + (pid->ki * pid->integral * PID_PERIOD) + (pid->kd * der / PID_PERIOD);

    //保存上一次误差
    pid->last_err = pid->err;
}

//串级PID计算
void Com_PID_Calc_Chain(PID_Struct *out_pid, PID_Struct *in_pid)
{
    //先计算外环
    Com_PID_Calc(out_pid);

    //将外环输出值作为内环目标值
    in_pid->desire = out_pid->output;

    //计算内环
    Com_PID_Calc(in_pid);
}

//限制数值在正常范围内
int16_t Com_limit(int16_t speed,int16_t max_speed,int16_t min_speed)
{
    if(speed > max_speed)
    {
        return max_speed;
    }
    else if(speed < min_speed)
    {
        return min_speed;
    }
    return speed;
}
