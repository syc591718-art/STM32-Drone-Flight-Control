#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h>

//日志输出打印在CPU运行上非常占用资源 = > 通过比特率计算
//所以在后续飞机需要正常飞行的时候 需要关闭打印功能
//设计一个日志输出打印开关
#define DEBUG_ON 1

#ifdef DEBUG_ON

//使用宏定义的方式 只打印文件名称 不打印路径名称
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#define __FILENAME (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILENAME__)
//使用宏定义的方式实现打印日志之前 先添加文件名和行号
#define debug_printf(format,...) printf("[%s:%d]  "  format, __FILENAME__, __LINE__, ##__VA_ARGS__)

#else
#define debug_printf(format,...)
#endif
#endif
