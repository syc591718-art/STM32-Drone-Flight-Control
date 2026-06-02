#include "Int_mpu6050.h"

//保存偏移量
int32_t acc_x_offset = 0;
int32_t acc_y_offset = 0;
int32_t acc_z_offset = 0;
int32_t gyro_x_offset = 0;
int32_t gyro_y_offset = 0;
int32_t gyro_z_offset = 0;

//写寄存器
void Int_MPU6050_Write_Reg(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1,MPU6050_ADDR_WRITE,reg,I2C_MEMADD_SIZE_8BIT,&data,1,1000);
}

//读寄存器
void Int_MPU6050_Read_Reg(uint8_t reg,uint8_t *data)
{
    HAL_I2C_Mem_Read(&hi2c1,MPU6050_ADDR_READ,reg,I2C_MEMADD_SIZE_8BIT,data,1,1000);
}

//初始化后零偏校准
void Int_MPU6050_calculate_offset(void)
{
    //等待飞机停放平稳
    //判断飞机是否停放平稳：前后两次加速度的值差值小于200，连续100次
    Accel_Struct current_accel = {0};
    Accel_Struct last_accel = {0};
    uint8_t count = 0;
    Int_MPU6050_Get_Acc(&last_accel);

    while(count < 100)
    {
        Int_MPU6050_Get_Acc(&current_accel);
        if(abs(current_accel.accel_x - last_accel.accel_x < 400) && abs(current_accel.accel_y - last_accel.accel_y < 400) && abs(current_accel.accel_z - last_accel.accel_z < 400))
        {
            count ++;
        }
        else
        {
            count = 0;
        }
        last_accel = current_accel;
        vTaskDelay(6);
    }

    //飞机平稳，开始零偏校准
    Gyro_Accel_Struct gyro_accel_data = {0};
    int32_t acc_x_sum = 0;
    int32_t acc_y_sum = 0;
    int32_t acc_z_sum = 0;
    int32_t gyro_x_sum = 0;
    int32_t gyro_y_sum = 0;
    int32_t gyro_z_sum = 0;
    for(uint8_t i = 0; i < 100; i++)
    {
        Int_MPU6050_Get_Data(&gyro_accel_data);
        acc_x_sum += (gyro_accel_data.accel.accel_x - 0);
        acc_y_sum += (gyro_accel_data.accel.accel_y - 0);
        acc_z_sum += (gyro_accel_data.accel.accel_z - 16384);

        gyro_x_sum += (gyro_accel_data.gyro.gyro_x - 0);
        gyro_y_sum += (gyro_accel_data.gyro.gyro_y - 0);
        gyro_z_sum += (gyro_accel_data.gyro.gyro_z - 0);

        vTaskDelay(6);
    }

    acc_x_offset = acc_x_sum / 100;
    acc_y_offset = acc_y_sum / 100;
    acc_z_offset = acc_z_sum / 100;
    gyro_x_offset = gyro_x_sum / 100;
    gyro_y_offset = gyro_y_sum / 100;
    gyro_z_offset = gyro_z_sum / 100;
}
//初始化MPU6050芯片
void Int_MPU6050_Init(void)
{
    //1.初始化I2C接口，main.c中已经完成

    //2.重启芯片，重置所有寄存器的值，写电源管理寄存器DEVICE_RESET（bit7）为1
    Int_MPU6050_Write_Reg(0x6B,0x80);
    uint8_t data = 0;
    //重置完成后，0x6B寄存器的值为0x40，表示当前为低功耗模式
    while(data != 0x40)
    {
        Int_MPU6050_Read_Reg(0x6B,&data);
    }
    //写0x00,唤醒MPU6050芯片
    Int_MPU6050_Write_Reg(0x6B,0x00);

    //3.选择合适的量程，在够用的范围内，量程越小越好
    //角速度量程±2000°/s
    Int_MPU6050_Write_Reg(0x1B,0x18);
    //加速度量程±2g
    Int_MPU6050_Write_Reg(0x1C,0x00);

    //4.关闭中断使能
    Int_MPU6050_Write_Reg(0x38,0x00);

    //5.用户配置寄存器，不使用FIFO队列，不使用扩展I2C
    Int_MPU6050_Write_Reg(0x6A,0x00);

    //6.设置采样频率，1000Hz
    //采样率必须大于两倍数据使用频率，否则失真
    //写为500Hz，设置采样分频为2,填写2-1
    Int_MPU6050_Write_Reg(0x19,0x01);

    //7.设置低通滤波值为184Hz（加速度） 188Hz（角速度）
    Int_MPU6050_Write_Reg(0x1A,0x01);

    //8.配置使用的系统时钟为添加PLL的
    Int_MPU6050_Write_Reg(0x6B,0x01);

    //9.使能加速度传感器和角速度传感器
    Int_MPU6050_Write_Reg(0x6C,0x00);

    //10.进行零偏校准
    Int_MPU6050_calculate_offset();
}

//读取三轴角速度
void Int_MPU6050_Get_Gyro(Gyro_Struct *gyro)
{
    uint8_t hight = 0;
    uint8_t low = 0;
    Int_MPU6050_Read_Reg(0x43,&hight);
    Int_MPU6050_Read_Reg(0x44,&low);
    gyro->gyro_x = ((hight << 8) | low) - gyro_x_offset;

    Int_MPU6050_Read_Reg(0x45,&hight);
    Int_MPU6050_Read_Reg(0x46,&low);
    gyro->gyro_y = ((hight << 8) | low) - gyro_y_offset;

    Int_MPU6050_Read_Reg(0x47,&hight);
    Int_MPU6050_Read_Reg(0x48,&low);
    gyro->gyro_z = ((hight << 8) | low) - gyro_z_offset;
}

//读取三轴加速度
void Int_MPU6050_Get_Acc(Accel_Struct *acc)
{
    uint8_t hight = 0;
    uint8_t low = 0;

    Int_MPU6050_Read_Reg(0x3B,&hight);
    Int_MPU6050_Read_Reg(0x3C,&low);
    acc->accel_x = ((hight << 8) | low) - acc_x_offset;

    Int_MPU6050_Read_Reg(0x3D,&hight);
    Int_MPU6050_Read_Reg(0x3E,&low);
    acc->accel_y = ((hight << 8) | low) - acc_y_offset;

    Int_MPU6050_Read_Reg(0x3F,&hight);
    Int_MPU6050_Read_Reg(0x40,&low);
    acc->accel_z = ((hight << 8) | low) - acc_z_offset;
}

//读取六轴数据
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data)
{
    Int_MPU6050_Get_Gyro(&data->gyro);
    Int_MPU6050_Get_Acc(&data->accel);
}
