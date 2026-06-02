#include "Int_joystick.h"

uint16_t adc_buff[4] = {0};
//初始化ADC摇杆
void Int_joystick_Init(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff,4);
}

//读取ADC数据，存入结构体地址中
void Int_joystick_get(Joystick_Struct *joystick_struct)
{
    joystick_struct->thr = adc_buff[0];
    joystick_struct->yaw = adc_buff[1];
    joystick_struct->pit = adc_buff[2];
    joystick_struct->rol = adc_buff[3];
}
