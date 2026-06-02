#include "App_transmit_data.h"

extern Remote_Data_Struct remote_data;
uint8_t transmit_buff[TX_PLOAD_WIDTH] = {0};
//打包发送遥控数据
void App_transmit_data(void)
{
    //切换为发送模式
    Int_SI24R1_TX_Mode();

    uint32_t sum = 0 ;
    //发送数据
    //帧头<<  2字节
    //数据本身10字节，高八位在前
    //校验和  4字节
    //帧尾>>  2字节
    transmit_buff[0] = FRAME_HEAD_CHECK;
    transmit_buff[1] = FRAME_HEAD_CHECK;

    transmit_buff[2] = (remote_data.thr >> 8) & 0xFF;
    transmit_buff[3] = remote_data.thr & 0xFF;
    transmit_buff[4] = (remote_data.yaw >> 8) & 0xFF;
    transmit_buff[5] = remote_data.yaw & 0xFF;
    transmit_buff[6] = (remote_data.pit >> 8) & 0xFF;
    transmit_buff[7] = remote_data.pit & 0xFF;
    transmit_buff[8] = (remote_data.rol >> 8) & 0xFF;
    transmit_buff[9] = remote_data.rol & 0xFF;

    taskENTER_CRITICAL();
    transmit_buff[10] = remote_data.shutdown;
    remote_data.shutdown = 0;
    transmit_buff[11] = remote_data.fix_height;
    remote_data.fix_height = 0;
    taskEXIT_CRITICAL();

    for (uint8_t i = 0; i <12; i++)
    {
        sum += transmit_buff[i];
    }
    transmit_buff[12] = (sum >> 24) & 0xFF;
    transmit_buff[13] = (sum >> 16) & 0xFF;
    transmit_buff[14] = (sum >> 8) & 0xFF;
    transmit_buff[15] = sum & 0xFF;

    transmit_buff[16] = FRAME_TAIL_CHECK;
    transmit_buff[17] = FRAME_TAIL_CHECK;

    uint8_t res = Int_SI24R1_TxPacket(transmit_buff);
    debug_printf("Tx res: %d\r\n", res);

    //切换为接受模式
    Int_SI24R1_RX_Mode();
}
