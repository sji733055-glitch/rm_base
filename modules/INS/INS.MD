# INS 姿态解算模块文档

## 概述

INS (Inertial Navigation System) 姿态解算模块是一个基于扩展卡尔曼滤波器(EKF)的惯性导航系统，用于实时计算设备的姿态角（Roll、Pitch、Yaw）。该模块集成了BMI088 IMU传感器，通过传感器数据融合算法提供高精度的姿态信息。

该实现基于Wang Hongxi在RoboMaster机器人中的开源姿态解算方案，结合了扩展卡尔曼滤波、四元数姿态表示和传感器融合技术。

## 特性

- 基于四元数的扩展卡尔曼滤波姿态估计算法
- 支持BMI088 IMU传感器数据采集与处理
- 实时姿态角计算（Roll、Pitch、Yaw）
- 陀螺仪零偏自动估计与补偿
- 加速度计数据用于姿态修正
- 温度控制与补偿
- 基于RTOS线程的后台处理机制
- 卡方检验提高算法鲁棒性
- 自适应增益调节

## 数据结构

### INS_t

```c
typedef struct
{
    float q[4];                    // 四元数估计值
    float MotionAccel_b[3];        // 机体坐标加速度
    float MotionAccel_n[3];        // 绝对系加速度
    float AccelLPF;                // 加速度低通滤波系数
    float xn[3], yn[3], zn[3];     // bodyframe在绝对系的向量表示
    IMU_Estimate_t IMU_Estimate;   // 位姿估计值
    IMU_Instance_t IMU;            // imu实例
    float dt;                      // 采样时间间隔
    uint32_t dwt_cnt;              // DWT计数器
} INS_t;
```

### INS_Param_t

```c
typedef struct
{
    uint8_t flag;        // 参数更新标志
    float scale[3];      // 三轴标度因子
    float Yaw;           // Yaw安装误差角
    float Pitch;         // Pitch安装误差角
    float Roll;          // Roll安装误差角
} INS_Param_t;
```

### QEKF_INS_t

```c
typedef struct
{
    uint8_t Initialized;              // 初始化标志
    KalmanFilter_t IMU_QuaternionEKF; // EKF滤波器实例
    uint8_t ConvergeFlag;             // 收敛标志
    uint8_t StableFlag;               // 稳定状态标志
    uint64_t ErrorCount;              // 错误计数
    uint64_t UpdateCount;             // 更新计数

    float q[4];          // 四元数估计值
    float GyroBias[3];   // 陀螺仪零偏估计值
    float Gyro[3];       // 陀螺仪数据
    float Accel[3];      // 加速度计数据
    float OrientationCosine[3]; // 方向余弦

    float accLPFcoef;    // 加速度低通滤波系数
    float gyro_norm;     // 陀螺仪模长
    float accl_norm;     // 加速度计模长
    float AdaptiveGainScale; // 自适应增益比例

    float Roll, Pitch, Yaw; // 欧拉角
    float YawTotalAngle;    // 累计Yaw角度

    float Q1, Q2, R;     // EKF过程噪声和观测噪声
    float dt;            // 姿态更新周期
    float ChiSquare_Data[1];      // 卡方检验检测函数
    float ChiSquareTestThreshold; // 卡方检验阈值
    float lambda;        // 渐消因子

    int16_t YawRoundCount; // Yaw角度圈数计数
    float YawAngleLast;    // 上次Yaw角度值
} QEKF_INS_t;
```

## 配置参数

```c
#define INS_THREAD_PRIORITY     优先级值     // INS线程优先级
#define INS_THREAD_STACK_SIZE   字节数       // INS线程栈大小
#define INIT_SAMPLE_COUNT       100          // 初始化采样次数
#define INS_ACCEL_LPF_COEF      0.0085f      // 加速度低通滤波系数
```

## 工作原理

### 初始化流程

1. 初始化IMU硬件设备
2. 初始化IMU参数修正结构体
3. 通过加速度计数据初始化四元数
4. 初始化扩展卡尔曼滤波器
5. 设置加速度低通滤波系数
6. 创建并启动INS任务线程

### 姿态解算流程

1. **数据采集**：从BMI088传感器获取加速度和角速度数据
2. **参数修正**：根据安装误差对传感器数据进行修正
3. **EKF更新**：使用扩展卡尔曼滤波器融合传感器数据
4. **姿态计算**：从四元数解算欧拉角（Roll、Pitch、Yaw）
5. **零偏估计**：实时估计并补偿陀螺仪零偏
6. **坐标变换**：实现机体坐标系与地球坐标系之间的向量转换

### EKF算法特点

- 使用四元数避免欧拉角奇异问题
- 估计陀螺仪零偏并进行实时补偿
- 使用加速度计数据对姿态进行修正
- 采用卡方检验确保数据融合的可靠性
- 实现自适应增益调节提高算法鲁棒性
- 通过渐消因子防止协方差矩阵过度收敛

### 算法参考

本模块基于以下开源项目和文献：

1. [RoboMaster机器人姿态解算方案开源](https://zhuanlan.zhihu.com/p/540676773)
2. [WangHongxi2001/RoboMaster-C-Board-INS-Example](https://github.com/WangHongxi2001/RoboMaster-C-Board-INS-Example)
3. [HNUYueLuRM/basic_framework](https://gitee.com/hnuyuelurm/basic_framework)

## 线程安全

INS模块在独立的RTOS线程中运行

## 注意事项

1. **初始化时机**：确保在IMU硬件初始化完成后再初始化INS模块
2. **数据同步**：访问姿态数据时注意线程同步问题
3. **传感器校准**：使用前应确保IMU传感器已完成校准
4. **温度影响**：长时间运行时注意温度对传感器精度的影响
5. **安装方向**：确保IMU安装方向与软件配置一致
6. **静态初始化**：系统启动时应保持IMU静止以便正确初始化姿态

## 错误处理

模块通过以下方式处理错误：

1. **初始化检查**：检查指针有效性，避免空指针访问
2. **传感器状态**：监控IMU初始化状态，失败时记录日志
3. **数据校验**：对传感器数据进行有效性检查
4. **日志记录**：通过日志系统记录错误信息，便于调试和问题排查