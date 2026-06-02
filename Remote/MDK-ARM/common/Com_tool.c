#include "Com_tool.h"

//限制函数
int16_t Com_limit(int16_t value,int16_t min,int16_t max)
{
    int16_t result = value;
    if(value > max)
    {
        result = max;
    }
    else if(value < min)
    {
        result = min;
    }
    return result;
}
