#ifndef __INT_JOYSTICK_H__
#define __INT_JOYSTICK_H__

#include "adc.h"

typedef struct 
{
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
}Joystick_Struct;

//初始化ADC摇杆
void Int_joystick_Init(void);

//读取ADC数据，存入结构体地址中
void Int_joystick_get(Joystick_Struct *joystick_struct);

#endif
