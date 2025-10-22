# BOARD_COMM 板间通信模块文档

## 概述

BOARD_COMM 板间通信模块是一个基于 CAN 总线的双板间通信系统，用于在云台板和底盘板之间传输控制指令和状态数据。该模块通过 CAN 总线实现板间数据交换，并集成了离线检测功能，确保通信的可靠性。

## 特性

- 基于 CAN 总线的可靠通信
- 支持云台板和底盘板之间的双向数据传输
- 集成离线检测功能
- 根据不同板卡类型提供不同的接口函数
- 支持数据编码和解码功能
- 基于 OSAL（操作系统抽象层）的跨平台设计

## 数据结构

### board_comm_t

```c
typedef struct {
    Can_Device *candevice;        // CAN设备指针
    uint8_t offlinemanage_index;  // 离线管理索引
} board_comm_t;
```
## 状态定义
```c
#define GIMBAL_ID    0X310        // 云台板ID
#define CHASSIS_ID   0X311        // 底盘板ID
```
## API 接口
```c
osal_status_t board_com_init(board_comm_t *board_comm);
```
```c
// 这是云台板定义下
osal_status_t board_com_send(const Chassis_Ctrl_Cmd_s *cmd);
osal_status_t board_com_recv(Chassis_Upload_Data_s *data);
```
```c
// 这是底盘板定义下
osal_status_t board_com_recv(Chassis_Ctrl_Cmd_s *cmd);
osal_status_t board_com_send(const Chassis_Upload_Data_s *data);
```

## 使用示例

### 云台板使用示例
```c
// 定义板间通信结构体
static board_comm_t board_comm;

// 初始化板间通信模块
if (board_com_init(&board_comm) != OSAL_SUCCESS) {
    // 初始化失败处理
    return;
}

// 发送底盘控制命令
Chassis_Ctrl_Cmd_s chassis_cmd = {0};
chassis_cmd.vx = 1.0f;
chassis_cmd.vy = 0.5f;
chassis_cmd.wz = 0.2f;
chassis_cmd.offset_angle = 45.0f;
chassis_cmd.chassis_mode = CHASSIS_FOLLOW_GIMBAL_YAW;
chassis_cmd.ui_flag_1 = 1;
chassis_cmd.ui_flag_2 = 0;

if (board_com_send(&chassis_cmd) != OSAL_SUCCESS) {
    // 发送失败处理
}

// 接收底盘上传数据
Chassis_Upload_Data_s chassis_data;
if (board_com_recv(&chassis_data) == OSAL_SUCCESS) {
    // 处理接收到的数据
}
```
### 底盘板使用示例
```c
// 定义板间通信结构体
static board_comm_t board_comm;

// 初始化板间通信模块
if (board_com_init(&board_comm) != OSAL_SUCCESS) {
    // 初始化失败处理
    return;
}

// 接收底盘控制命令
Chassis_Ctrl_Cmd_s chassis_cmd;
if (board_com_recv(&chassis_cmd) == OSAL_SUCCESS) {
    // 处理接收到的控制命令
    float vx = chassis_cmd.vx;
    float vy = chassis_cmd.vy;
    float wz = chassis_cmd.wz;
    // 执行相应的底盘控制逻辑
}

// 发送底盘上传数据
Chassis_Upload_Data_s chassis_data = {0};
chassis_data.reversed_1 = 100;
chassis_data.reversed_2 = 50;
// 设置其他字段...

if (board_com_send(&chassis_data) != OSAL_SUCCESS) {
    // 发送失败处理
}
```
## 工作原理
### 初始化流程
1. 根据编译宏定义（GIMBAL_BOARD 或 CHASSIS_BOARD）配置CAN设备参数
2. 初始化CAN设备，设置收发ID
3. 注册离线检测设备，监控板间通信状态
### 数据传输机制
1. 云台板到底盘板：云台板将Chassis_Ctrl_Cmd_s结构体编码为8字节CAN数据发送
2. 底盘板到云台板：底盘板将Chassis_Upload_Data_s结构体内存拷贝到CAN数据发送
### 离线检测集成
1. 初始化时注册为离线检测设备，超时时间为100ms
2. 离线等级为高优先级（OFFLINE_LEVEL_HIGH）
3. 配置蜂鸣器报警次数为7次
## 注意事项
1. 编译配置：必须正确定义GIMBAL_BOARD或CHASSIS_BOARD宏，以启用相应功能
2. 数据精度：速度值传输时会转换为int8_t类型，精度范围为-128到127
5. 离线检测：模块集成了离线检测功能，可监控通信状态
## 错误处理
### 模块通过以下方式处理错误：
1. 初始化错误：CAN设备初始化失败或离线检测注册失败时返回OSAL_ERROR
2. 参数错误：输入参数为NULL时返回OSAL_ERROR
3. 通信错误：CAN通信失败时返回OSAL_ERROR
4. 系统错误：通过日志系统记录错误信息，便于调试和问题排查