#include "Int_SI24R1.h"

// 定义一个静态的发送地址
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x0A, 0x01, 0x07, 0x01, 0x08};

// SPI读写一字节
static uint8_t SPI_RW(uint8_t byte)
{
	uint8_t rx_data = 0;
	HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_data, 1, 1000);
	return rx_data;
}

// 写寄存器的值（单字节）
// 参数：reg:SI24R1_WRITE_REG | reg
uint8_t Int_SI24R1_Write_Reg(uint8_t reg, uint8_t value)
{
	uint8_t status;

	CS_LOW;
	status = SPI_RW(reg);
	SPI_RW(value);
	CS_HIGH;

	return (status);
}
// 写寄存器的值（多字节）
// 参数：reg:SI24R1_WRITE_REG | reg
uint8_t Int_SI24R1_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t size)
{
	uint8_t status, byte_ctr;
	CS_LOW;
	status = SPI_RW(reg);
	for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
	{
		SPI_RW(pBuf[byte_ctr]);
	}
	CS_HIGH;

	return (status);
}

// 读寄存器的值（单字节）
// 参数：reg:SI24R1_READ_REG | reg
uint8_t Int_SI24R1_Read_Reg(uint8_t reg)
{
	uint8_t value;

	CS_LOW;
	SPI_RW(reg);
	value = SPI_RW(0);
	CS_HIGH;

	return (value);
}

// 读寄存器的值（多字节）
// 参数：reg:SI24R1_READ_REG | reg
uint8_t Int_SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t size)
{
	uint8_t status, byte_ctr;

	CS_LOW;
	status = SPI_RW(reg);
	for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
	{
		pBuf[byte_ctr] = SPI_RW(0);
	}
	CS_HIGH;

	return (status);
}

// 进入接收模式
void Int_SI24R1_RX_Mode(void)
{
	CE_LOW;
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 设置通道0接收地址
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);						   // 使能通道0自动应答
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);					   // 使能接收通道0
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, 40);							   // 选择射频通道40
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);			   // 接收通道0的接收数据长度
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);					   // 设置射频速率为1Mbps，TX发射功率为4dBm
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0f);						   // 使能CRC，长度为2字节，开机模式，接受模式
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, 0xff);						   // 清除中断标志位
	CE_HIGH;
}

// 进入发送模式
void Int_SI24R1_TX_Mode(void)
{
	CE_LOW;
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);	   // 设置发送地址
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 设置通道0接收地址
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);						   // 使能通道0自动应答
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);					   // 使能接收通道0
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + SETUP_RETR, 0x0a);					   // 自动重发延时等待250us，自动重发次数为10次
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, 40);							   // 选择射频通道40
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);					   // 设置射频速率为1Mbps，TX发射功率为4dBm
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0e);						   // 使能CRC，长度为2字节，开机模式，发送模式
	CE_HIGH;
}

// 读取接收数据
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf)
{
	uint8_t state;
	state = Int_SI24R1_Read_Reg(STATUS);
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state);//清除RX_DR中断标志

	if (state & RX_DR)
	{
		Int_SI24R1_Read_Buf(RD_RX_PLOAD, rxbuf, TX_PLOAD_WIDTH);
		Int_SI24R1_Write_Reg(FLUSH_RX, 0xff);
		return 0;//收到数据
	}
	return 1;//没收到数据
}

// 发送一个数据包
//返回0表示发送成功，1表示发送失败
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf)
{
	uint8_t state;
	CE_LOW;
	Int_SI24R1_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
	CE_HIGH;

	//使用轮询读取标志位
	state = Int_SI24R1_Read_Reg(STATUS);
	while(((state & TX_DS) == 0) && ((state & MAX_RT) == 0))
	{
		state = Int_SI24R1_Read_Reg(STATUS);
		vTaskDelay(1);
	}
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state);
	if (state & MAX_RT)
	{
		Int_SI24R1_Write_Reg(FLUSH_TX, 0xff);
		return 1;
	}
	if (state & TX_DS)
	{
		return 0;
	}
	return 1;
}

uint8_t SI24R1_rx_buff[TX_PLOAD_WIDTH]= {0};

uint8_t Int_SI24R1_Check(void)
{
	//测试SPI通讯能够正常写寄存器
	//写入前要先读取一次，保证SPI正常再写入
	Int_SI24R1_Read_Buf(TX_ADDR, SI24R1_rx_buff, TX_ADR_WIDTH);
	//写入
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
	//读取
	Int_SI24R1_Read_Buf(TX_ADDR, SI24R1_rx_buff, TX_ADR_WIDTH);

	for(int i = 0; i<TX_ADR_WIDTH; i ++)
	{
		if(SI24R1_rx_buff[i] != TX_ADDRESS[i])
		{
			return 1;//检测失败
		}
	}
	return 0;//检测成功
}

//硬件接口层初始化
void Int_SI24R1_Init(void)
{
	HAL_Delay(200);
	while(Int_SI24R1_Check())
	{
		HAL_Delay(10);
	}

	//设置默认的状态为接受模式
	Int_SI24R1_RX_Mode();
	debug_printf("SI24R1_Init success\r\n");
}
