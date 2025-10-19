# USB 用户层接口文档

## 概述

USB 用户层接口模块提供了一套用户友好的 API，用于通过 USB CDC ACM（Communication Device Class - Abstract Control Model）与上位机进行数据通信。该模块基于 BSP 层的 USB CDC 和CherryUSB实现，并集成了系统的离线检测机制，可以监控 USB 通信状态。

## 特性

- 简单易用的 API 接口
- 自动集成离线检测机制
- 数据帧格式验证（帧头和帧尾）
- 支持自定义数据结构传输
- 基于双缓冲机制的数据接收
- 线程安全的操作接口
  
  ## 数据结构
  
  ### User_Send_s
  
  ```c
  struct User_Send_s
  {
    uint8_t header;   // 帧头标识，固定为0xAA
    uint8_t mode;     // 工作模式
    float roll;       // 滚转角
    float pitch;      // 俯仰角
    float yaw;        // 偏航角
    uint8_t end;      // 帧尾标识，固定为0xBB
  };
  ```
  
  ### User_Recv_s
  
  ```c
  struct User_Recv_s
  {
    uint8_t header;       // 帧头标识，固定为0xAA
    uint8_t fire_advice;  // 射击建议
    float pitch;          // 俯仰角
    float yaw;            // 偏航角
    float distance;       // 距离
    uint8_t end;          // 帧尾标识，固定为0xBB
  };
  ```
  
  ### user_cdc_t
  
  ```c
  typedef struct{
    USB_CDC_Device *cdc_device;  // CDC 设备指针
    uint8_t offline_index;       // 离线检测设备索引
  } user_cdc_t;
  ```
  
  ## API 接口
  
  ```c
  osal_status_t usb_user_init(user_cdc_t *instance);
  ```
  
  ```c
  osal_status_t usb_user_send(struct User_Send_s *send);
  ```
  
  ```c
  struct User_Recv_s* usb_user_recv();
  ```
  
  ## 使用示例
  
  ```c
  #include "usb_user.h"
  #include "offline.h"
  #include <string.h>
  
  // 定义USB CDC用户设备实例
  static user_cdc_t usb_cdc_instance;
  
  // 定义发送数据
  struct User_Send_s send_data = {
      .header = 0xAA,
      .mode = 1,
      .roll = 0.0f,
      .pitch = 0.0f,
      .yaw = 0.0f,
      .end = 0xBB
  };
  
  int main(void)
  {
      // 初始化USB用户设备
      if (usb_user_init(&usb_cdc_instance) != OSAL_SUCCESS) {
          // 初始化失败处理
          return -1;
      }
      
      // 发送数据
      if (usb_user_send(&send_data) != OSAL_SUCCESS) {
          // 发送失败处理
          return -1;
      }
      
      // 接收数据
      struct User_Recv_s* recv_data = usb_user_recv();
      if (recv_data != NULL) {
          // 处理接收到的数据
          // ...
      }
      
      return 0;
  }
  ```
  
  ## 工作原理
  
  ### 初始化流程
  1. 调用 usb_user_init 初始化 USB CDC 设备
  2. 函数内部调用 cdc_acm_init 初始化底层 USB CDC 驱动
  3. 注册设备到离线检测模块，用于监控通信状态
  
  ### 数据发送流程
  
  1. 用户调用 usb_user_send 发送数据
  2. 函数内部调用 cdc_acm_send 发送数据到上位机
  
  ### 数据接收流程
  
  1. 用户调用 usb_user_recv 接收数据
  2. 函数内部调用 cdc_acm_read 从 USB 接收数据
  3. 验证数据帧格式（帧头 0xAA 和帧尾 0xBB）
  4. 如果数据有效，更新离线检测状态并返回数据指针
  
  ## 数据帧格式
  
  ### 发送帧格式
  
  | 字段     | 类型      | 长度(字节) | 说明       |
  | ------ | ------- | ------ | -------- |
  | header | uint8_t | 1      | 帧头(0xAA) |
  | mode   | uint8_t | 1      | 工作模式     |
  | roll   | float   | 4      | 滚转角      |
  | pitch  | float   | 4      | 俯仰角      |
  | yaw    | float   | 4      | 偏航角      |
  | end    | uint8_t | 1      | 帧尾(0xBB) |
  | **总计** |         | **15** |          |
  
  ### 接收帧格式
  
  | 字段          | 类型      | 长度(字节) | 说明       |
  | ----------- | ------- | ------ | -------- |
  | header      | uint8_t | 1      | 帧头(0xAA) |
  | fire_advice | uint8_t | 1      | 射击建议     |
  | pitch       | float   | 4      | 俯仰角      |
  | yaw         | float   | 4      | 偏航角      |
  | distance    | float   | 4      | 距离       |
  | end         | uint8_t | 1      | 帧尾(0xBB) |
  | **总计**      |         | **15** |          |
  
  ## 离线检测集成
  
  USB 用户层接口自动集成了系统的离线检测机制：
  
  1. 在初始化时自动注册到离线检测模块
  2. 每次成功接收数据时自动更新设备状态
  3. 当超过设定时间（100ms）未接收到有效数据时，设备会被标记为离线
  4. 离线时可根据配置触发报警（如蜂鸣器、RGB 灯等）
  
  ## 注意事项
  
  1. **内存管理**：用户不需要手动管理 USB 接收缓冲区，模块内部使用双缓冲机制
  2. **线程安全**：所有 API 都是线程安全的，可以在多个线程中调用
  3. **数据验证**：接收数据时会自动验证帧头和帧尾，确保数据完整性
  4. **错误处理**：所有函数都有明确的返回值，用户应检查返回值以确保操作成功
  5. **离线检测**：模块自动集成离线检测机制，用户无需额外操作
  
  ## 错误处理
  
  模块通过以下方式处理错误：
  
  1. **初始化失败**：当 USB 设备初始化失败或离线检测注册失败时，返回 `OSAL_ERROR`
  2. **参数错误**：当传递给函数的参数为 NULL 时，返回相应错误码
  3. **发送失败**：当底层发送失败时，返回 `OSAL_ERROR`
  4. **接收失败**：当接收数据格式不正确时，返回 NULL
  5. **系统错误**：通过日志系统记录错误信息，便于调试和问题排查
  
  
  
