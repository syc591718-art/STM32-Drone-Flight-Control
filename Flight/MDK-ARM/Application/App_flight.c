#include "App_flight.h"

Gyro_Accel_Struct gyro_accel_data = {0};
Euler_Struct euler_angle = {0};
Gyro_Struct last_gyro = {0};
float gyro_z_sum_count = 0;

extern Remote_Data_Struct remote_data;
extern Flight_State flight_state;
extern uint16_t fix_height;
extern TaskHandle_t COM_task_handle;

//电机结构体
Motor_Struct left_top_motor ={.tim = &htim3, .channel = TIM_CHANNEL_1, .speed = 0};
Motor_Struct right_top_motor ={.tim = &htim2, .channel = TIM_CHANNEL_2, .speed = 0};
Motor_Struct left_bottom_motor ={.tim = &htim4, .channel = TIM_CHANNEL_4, .speed = 0};
Motor_Struct right_bottom_motor ={.tim = &htim1, .channel = TIM_CHANNEL_3, .speed = 0};

//俯仰角PID结构体
PID_Struct pit_pid = {.kp = -7.00,.ki = 0.00, .kd = 0.00};
//Y轴角速度结构体，对应俯仰角的内环
PID_Struct gyro_y_pid = {.kp = 3.00,.ki = 0.00, .kd = 0.50};

//横滚角PID结构体
PID_Struct rol_pid = {.kp = -7.00,.ki = 0.00, .kd = 0.00};
//X轴角速度结构体，对应横滚角的内环
PID_Struct gyro_x_pid = {.kp = 3.00,.ki = 0.00, .kd = 0.50};

//偏航角PID结构体
PID_Struct yaw_pid = {.kp = -3.00,.ki = 0.00, .kd = 0.00};
//Z轴角速度结构体，对应偏航角的内环
PID_Struct gyro_z_pid = {.kp = -5.00,.ki = 0.00, .kd = 0.00};

//定高的PID结构体
PID_Struct height_pid = {.kp = -0.60,.ki = 0.00, .kd = -0.20};

//飞控任务初始化：MPU6050初始化、电机初始化
void App_flight_init(void)
{
    //初始化MPU6050
    Int_MPU6050_Init();

    //初始化电机
    Int_motor_start(&left_top_motor);
    Int_motor_start(&right_top_motor);
    Int_motor_start(&left_bottom_motor);
    Int_motor_start(&right_bottom_motor);

    //初始化激光测距仪
    Int_VL53L1X_Init();
}

//根据陀螺仪测量的数据计算欧拉角
void App_flight_get_euler_angle(void)
{
    //获取六轴数据
    Int_MPU6050_Get_Data(&gyro_accel_data);

    //对角速度进行低通滤波，对数据采集的使用需要及时性高，对准确性不高
    //output = 加权系数 * last_output + (1 - 加权系数) * 本次测量值
    gyro_accel_data.gyro.gyro_x = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_x,last_gyro.gyro_x);
    gyro_accel_data.gyro.gyro_y = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_y,last_gyro.gyro_y);
    gyro_accel_data.gyro.gyro_z = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_z,last_gyro.gyro_z);
    last_gyro.gyro_x = gyro_accel_data.gyro.gyro_x;
    last_gyro.gyro_y = gyro_accel_data.gyro.gyro_y;
    last_gyro.gyro_z = gyro_accel_data.gyro.gyro_z;

    //对波动变化比较大的加速度，使用卡尔曼滤波
    gyro_accel_data.accel.accel_x = Common_Filter_KalmanFilter(&kfs[0], gyro_accel_data.accel.accel_x);
    gyro_accel_data.accel.accel_y = Common_Filter_KalmanFilter(&kfs[1], gyro_accel_data.accel.accel_y);
    gyro_accel_data.accel.accel_z = Common_Filter_KalmanFilter(&kfs[2], gyro_accel_data.accel.accel_z);

    //通过加速度和角速度来计算当前飞机倾斜角度->姿态解算
    //互补解算计算欧拉角
    //俯仰角和横滚角使用加速度解算
    //偏航角使用角速度积分
    //euler_angle.pit = atan2(gyro_accel_data.accel.accel_x * 1.0, gyro_accel_data.accel.accel_z) /3.14159 * 180;
    //euler_angle.rol = atan2(gyro_accel_data.accel.accel_y * 1.0, gyro_accel_data.accel.accel_z) /3.14159 * 180;
    
    //转换为2000°/s
    //gyro_z_sum_count += (gyro_accel_data.gyro.gyro_z * 2000 / 32768.0) * 0.006;
    //euler_angle.yaw = gyro_z_sum_count;

    //用移植的四元数姿态解算
    Common_IMU_GetEulerAngle(&gyro_accel_data, &euler_angle, 0.006);
}

//根据欧拉角计算出PID的目标值
void App_flight_pid_process(void)
{
    //俯仰角，需要赋值目标值和测量值
    //外环的目标角度：平稳飞行，值为0；遥控飞行，目标角度是遥控器的值
    //数值转换，0~1000要减500.控制范围在±10°
    pit_pid.desire = (remote_data.pit - 500) / 50.0;
    pit_pid.measure = euler_angle.pit;
    gyro_y_pid.measure = (gyro_accel_data.gyro.gyro_y * 2000.0 / 32768.0);
    Com_PID_Calc_Chain(&pit_pid, &gyro_y_pid);

    //横滚角，需要赋值目标值和测量值
    rol_pid.desire = (remote_data.rol - 500) / 50.0;
    rol_pid.measure = euler_angle.rol;
    gyro_x_pid.measure = (gyro_accel_data.gyro.gyro_x * 2000.0 / 32768.0);
    Com_PID_Calc_Chain(&rol_pid, &gyro_x_pid);

    //偏航角，需要赋值目标值和测量值
    yaw_pid.desire = (remote_data.yaw - 500) / 50.0;
    yaw_pid.measure = euler_angle.yaw;
    gyro_z_pid.measure = (gyro_accel_data.gyro.gyro_z * 2000.0 / 32768.0);
    Com_PID_Calc_Chain(&yaw_pid, &gyro_z_pid);

    //debug_printf(":%.2f,%.2f\n", gyro_y_pid.err,gyro_y_pid.output);
}

//根据PID的输出值控制电机
void App_flight_control_motor(void)
{
    //首先判断飞行状态
    switch(flight_state)
    {
        case IDLE:
            left_top_motor.speed = 0;
            right_top_motor.speed = 0;
            left_bottom_motor.speed = 0;
            right_bottom_motor.speed = 0;
            break;
        case NORMAL:
            //俯仰角->向前飞有角速度->正误差->需要向后飞抵消误差->前两个电机转的快，后两个电机转的慢
            left_top_motor.speed = remote_data.thr + gyro_y_pid.output - gyro_x_pid.output + Com_limit(gyro_z_pid.output,100,-100);
            right_top_motor.speed = remote_data.thr + gyro_y_pid.output + gyro_x_pid.output - Com_limit(gyro_z_pid.output,100,-100);
            left_bottom_motor.speed = remote_data.thr - gyro_y_pid.output - gyro_x_pid.output - Com_limit(gyro_z_pid.output,100,-100);
            right_bottom_motor.speed = remote_data.thr - gyro_y_pid.output + gyro_x_pid.output + Com_limit(gyro_z_pid.output,100,-100);
            break;
        case FIX_HEIGHT:
            left_top_motor.speed = remote_data.thr + gyro_y_pid.output - gyro_x_pid.output + Com_limit(gyro_z_pid.output,100,-100) + height_pid.output;
            right_top_motor.speed = remote_data.thr + gyro_y_pid.output + gyro_x_pid.output - Com_limit(gyro_z_pid.output,100,-100) + height_pid.output;
            left_bottom_motor.speed = remote_data.thr - gyro_y_pid.output - gyro_x_pid.output - Com_limit(gyro_z_pid.output,100,-100) + height_pid.output;
            right_bottom_motor.speed = remote_data.thr - gyro_y_pid.output + gyro_x_pid.output + Com_limit(gyro_z_pid.output,100,-100) + height_pid.output;
            break;
        case FAIL:
            //进行故障处理
            //每6ms，速度降低3点
            left_top_motor.speed -= 3;
            right_top_motor.speed -= 3;
            left_bottom_motor.speed -= 3;
            right_bottom_motor.speed -= 3;
            if(left_top_motor.speed <= 0 && right_top_motor.speed <= 0 && left_bottom_motor.speed <= 0 && right_bottom_motor.speed <= 0)
            {
                //故障处理完成，电机转速均为0，发送直接任务通知
                xTaskNotifyGive(COM_task_handle);
            }
            break;
        default:
            break;
    }

    //限制电机速度的上限值
    left_top_motor.speed = Com_limit(left_top_motor.speed, 700, 0);
    right_top_motor.speed = Com_limit(right_top_motor.speed, 700, 0);
    left_bottom_motor.speed = Com_limit(left_bottom_motor.speed, 700, 0);
    right_bottom_motor.speed = Com_limit(right_bottom_motor.speed, 700, 0);

    //设置电机速度
    Int_motor_set_speed(&left_top_motor);
    Int_motor_set_speed(&right_top_motor);
    Int_motor_set_speed(&left_bottom_motor);
    Int_motor_set_speed(&right_bottom_motor);
}

//定高功能中PID计算
void App_flight_fix_height_pid_process(void)
{
    //24ms一次
    //填写目标值（按下定高功能时的高度）和测量值（当前测距仪的道德高度）
    height_pid.desire = fix_height;
    height_pid.measure = Int_VL53L1X_Get_Distance();

    Com_PID_Calc(&height_pid);
}
