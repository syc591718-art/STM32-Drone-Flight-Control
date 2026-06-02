#ifndef __APP_RECEIVE_DATA_H__
#define __APP_RECEIVE_DATA_H__

#include "Int_SI24R1.h"
#include "Com_config.h"
#include "Int_VL53L1X.h"

//帧头<<
#define FRAME_HEAD_CHECK '<'
//帧尾>>
#define FRAME_TAIL_CHECK '>'

//最大重试次数
#define MAX_RETRY_TIMES 5

//接受遥控器发送的数据
//0表示校验成功，接受正常；1表示失败
uint8_t App_receive_data(void);

//处理连接状态
void App_process_connect_state(uint8_t res);

//处理飞行状态
void App_process_flight_state(void);

#endif 
