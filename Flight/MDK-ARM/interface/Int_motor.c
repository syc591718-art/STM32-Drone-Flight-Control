#include "Int_motor.h"

//设置马达转速
//传入的参数是占空比相对的值，最大1000，默认200
void Int_motor_set_speed(Motor_Struct *motor)
{
    if(motor->speed > 1000)
    {
        debug_printf("speed is too large, max is 1000\r\n");
        return;
    }
    __HAL_TIM_SET_COMPARE(motor->tim,motor->channel,motor->speed);
}

//启动电机
void Int_motor_start(Motor_Struct *motor)
{
    __HAL_TIM_SET_COMPARE(motor->tim,motor->channel,0);
    HAL_TIM_PWM_Start(motor->tim,motor->channel);
}
