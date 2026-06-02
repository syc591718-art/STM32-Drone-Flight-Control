#ifndef __INT_KEY_H__
#define __INT_KEY_H__

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum{
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LEFT_X,
    KEY_RIGHT_X,
    KEY_RIGHT_X_LONG,
}Key_type;

//获取当前按键是否被按下
Key_type Int_Get_Key(void);
#endif
