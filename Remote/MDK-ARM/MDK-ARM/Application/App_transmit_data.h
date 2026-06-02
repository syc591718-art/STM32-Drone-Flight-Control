#ifndef __APP_TRANSMIT_DATA_H__
#define __APP_TRANSMIT_DATA_H__

#include "Int_SI24R1.h"
#include "App_process_data.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"

//帧头<<
#define FRAME_HEAD_CHECK '<'
//帧尾>>
#define FRAME_TAIL_CHECK '>'

//打包发送遥控数据
void App_transmit_data(void);

#endif
