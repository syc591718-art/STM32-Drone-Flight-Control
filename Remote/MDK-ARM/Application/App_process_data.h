#ifndef __APP_PROCESS_DATA_H__
#define __APP_PROCESS_DATA_H__

#include "Int_joystick.h"
#include "Int_key.h"
#include "Com_debug.h"
#include "Com_tool.h"

typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t rol;
    int16_t pit;
    uint8_t shutdown;   // 0:不关机 1:关机
    uint8_t fix_height; // 0:不切换 1:切换定高或不定高
} Remote_Data_Struct;

// 处理按键数据
void App_process_key_data(void);

// 处理摇杆数据
void App_process_joystick_data(void);

#endif
