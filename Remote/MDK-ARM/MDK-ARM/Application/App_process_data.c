#include "App_process_data.h"

void App_process_joystick_data(void);

//摇杆结构体
Joystick_Struct joystick = {0};

//遥控结构体
Remote_Data_Struct remote_data = {0};

//按键微调值
int16_t key_pit_offset = 0;
int16_t key_rol_offset = 0;

//摇杆零偏校准值
int16_t joystick_thr_offset = 0;
int16_t joystick_yaw_offset = 0;
int16_t joystick_pit_offset = 0;
int16_t joystick_rol_offset = 0;

//校准摇杆函数
void App_calibrate_joystick(void)
{
    //清空按键微调值
    key_pit_offset = 0;
    key_rol_offset = 0;

    //减去多次求取平均值的零偏
    int16_t thr_sum = 0;
    int16_t yaw_sum = 0;
    int16_t pit_sum = 0;
    int16_t rol_sum = 0;
    for (uint8_t i = 0; i < 10; i ++)
    {
        App_process_joystick_data();
        thr_sum += joystick.thr - 0;
        yaw_sum += joystick.yaw - 500;
        pit_sum += joystick.pit - 500;
        rol_sum += joystick.rol - 500;
        vTaskDelay(10);
    }
    joystick_thr_offset += thr_sum / 10;
    joystick_yaw_offset += yaw_sum / 10;
    joystick_pit_offset += pit_sum / 10;
    joystick_rol_offset += rol_sum / 10;
}

//处理按键数据
void App_process_key_data(void)
{
    //调用按键接口，获取按键状态
    Key_type key = Int_Get_Key();
    if (key == KEY_UP)
    {
        //向前，俯仰角+
        key_pit_offset += 10;
    }
    else if(key == KEY_DOWN)
    {
        //向后，俯仰角-
        key_pit_offset -= 10;
    }
    else if(key == KEY_LEFT)
    {
        //向左，横滚角-
        key_rol_offset -= 10;
    }
    else if(key == KEY_RIGHT)
    {
        //向右，横滚角+
        key_rol_offset += 10;
    }
    else if(key == KEY_LEFT_X)
    {
        remote_data.shutdown = 1;
    }
    else if(key == KEY_RIGHT_X)
    {
        remote_data.fix_height = 1;
    }
    else if(key == KEY_RIGHT_X_LONG)
    {
        App_calibrate_joystick();
    }
}

//处理摇杆数据
void App_process_joystick_data(void)
{
    taskENTER_CRITICAL();

    //调用读取函数
    Int_joystick_get(&joystick);

    //处理范围和极性：4095~0->0~1000
    joystick.thr = 1000 - joystick.thr * 1000 / 4095;
    joystick.yaw = 1000 - joystick.yaw * 1000 / 4095;
    joystick.pit = 1000 - joystick.pit * 1000 / 4095;
    joystick.rol = 1000 - joystick.rol * 1000 / 4095;

    //处理零偏校准
    joystick.thr -= joystick_thr_offset;
    joystick.yaw -= joystick_yaw_offset;
    joystick.pit -= joystick_pit_offset;
    joystick.rol -= joystick_rol_offset;

    //考虑按键微调值
    joystick.pit += key_pit_offset;
    joystick.rol += key_rol_offset;

    //限制范围0~1000
    joystick.thr = Com_limit(joystick.thr,0,1000);
    joystick.yaw = Com_limit(joystick.yaw,0,1000);
    joystick.pit = Com_limit(joystick.pit,0,1000);
    joystick.rol = Com_limit(joystick.rol,0,1000);

    taskEXIT_CRITICAL();

    remote_data.thr = joystick.thr;
    remote_data.yaw = joystick.yaw;
    remote_data.pit = joystick.pit;
    remote_data.rol = joystick.rol;

    debug_printf(":%d,%d,%d,%d\n",joystick.thr,joystick.yaw,joystick.pit,joystick.rol);
}
