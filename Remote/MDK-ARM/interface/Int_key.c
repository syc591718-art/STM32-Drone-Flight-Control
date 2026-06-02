#include "Int_key.h"

//获取当前按键是否被按下
Key_type Int_Get_Key(void)
{
    if(HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin) == GPIO_PIN_RESET)
    {
        //消除电弧抖动
        vTaskDelay(5);
        if(HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_UP;
        }
    }
    else if(HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if(HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_DOWN;
        }
    }
    else if(HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if(HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_LEFT;
        }
    }
    else if(HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if(HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_RIGHT;
        }
    }
    else if(HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port,KEY_LEFT_X_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if(HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port,KEY_LEFT_X_Pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port,KEY_LEFT_X_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_LEFT_X;
        }
    }
    else if(HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port,KEY_RIGHT_X_Pin) == GPIO_PIN_RESET)
    {
        //获取当前基准时钟
        TickType_t count1 = xTaskGetTickCount();

        vTaskDelay(5);
        if(HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port,KEY_RIGHT_X_Pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port,KEY_RIGHT_X_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            TickType_t count2 = xTaskGetTickCount();
            if(count2 - count1 > 1000)
            {
                return KEY_RIGHT_X_LONG;
            }
            else
            {
                return KEY_RIGHT_X;
            }
        }
    }
    return KEY_NONE;
}
