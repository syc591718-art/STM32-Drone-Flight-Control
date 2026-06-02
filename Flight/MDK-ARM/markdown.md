- Com_debug：重定向编写了打印函数debug_printf()
- Com_filter：两种滤波方法，对角速度低通滤波，对加速度卡尔曼滤波
- Com_imu：姿态解算工具，将角速度加速度转为欧拉角
- Com_pid：PID计算，根据欧拉角误差值计算PID值

- Int_IP5305T：软件层面模拟了IP5305T芯片的单次短按开机、两次长按关机
- 在App_freeRTOS_Task中，POWER_task使用直接任务通知，如果要关机，就运行xTaskNotifyGive，通过ulTaskNotifyTake接收

- Int_motor：有设置马达指定定时器、指定通道、速度和开启指定定时器、指定通道PWM输出的函数(带有电机结构体)
- 在App_freeRTOS_Task中，

- Int_led：有开灯、关灯、反转灯的函数（带有LED结构体）
- 在App_freeRTOS_Task中，LED_task说明灯1：左上、灯2：右上、灯3：右下、灯4：左下
前两个灯亮表示连接，前两个灯灭表示未连接
后两个灯慢闪表示空闲状态，快闪表示常规状态，持续亮表示定高状态，熄灭表示故障状态

- Int_SI24R1：有进入接收模式、进入发送模式，读取数据，发送数据，SI24R1初始化的函数
- App_receive_data：有接收遥控器发送的数据、处理连接状态、处理解锁逻辑、处理飞行状态的函数
- 在App_freeRTOS_Task中，COM接收数据，处理连接状态，处理关机命令，处理飞机飞行状态

- Int_mpu6050：有初始化mpu6050、读取六轴数据的函数
- App_flight：有计算欧拉角和PID值的功能
- 在App_freeRTOS_Task中，

- Int_key：有获取当前按键状态的函数，可以获取哪个按键被按下
- Int_joystick：有初始化摇杆，读取ADC数据的函数
- App_process_data：有校准摇杆数据，处理按键，处理摇杆数据的函数
- 在App_freeRTOS_Task中，KEY_task获取按键状态，JOYSTICK_task获取摇杆数据
- App_transmit_data：有发送遥控数据的函数
- 在App_freeRTOS_Task中，COM处理摇杆数据，处理按键数据，处理飞机飞行状态，处理关机命令

- thr:油门、pit:俯仰、yaw:偏航、rol:横滚
- 前后俯仰角，左右横滚角，旋转横滚角
- 