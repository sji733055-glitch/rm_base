# BSP USB CDC ACM 驱动文档

## 概述

BSP USB CDC ACM 模块是基于 CherryUSB 协议栈实现的 USB 通信设备类抽象控制模型驱动。该模块提供了底层的 USB CDC ACM 功能实现，包括设备初始化、数据发送和接收等核心功能。此 BSP 层驱动为上层应用提供了一套简洁易用的 API 接口。

## 特性

- 基于 CherryUSB 协议栈实现

- 支持 USB CDC ACM（Communication Device Class - Abstract Control Model）

- 双缓冲接收机制，提高数据处理效率

- 事件驱动模型，支持异步数据处理

- 线程安全的操作接口

- 可配置的接收缓冲区大小

- 支持指定长度数据包接收
  
  ## 数据结构
  
  ### USB_CDC_Device
  
  USB CDC 设备结构体，包含设备的所有状态和配置信息。
  
  ```c
  typedef struct {
      uint8_t busid;                    // 总线ID
      uintptr_t reg_base;               // 寄存器基地址
  
      // 接收相关
      uint8_t (*rx_buf)[2];             // 指向外部定义的双缓冲区
      uint16_t rx_buf_size;             // 缓冲区大小
      volatile uint8_t rx_active_buf;   // 当前活动缓冲区
      uint16_t real_rx_len;             // 实际接收数据长度
      uint16_t expected_rx_len;         // 预期长度（0为不定长）
  
      // USB CDC事件（用于接收和发送完成通知）
      osal_event_t usb_cdc_event;       // USB CDC事件组
  
      // 发送相关
      osal_sem_t ep_tx_sem;             // 发送端点信号量
  
      // 设备状态
      uint8_t is_initialized;           // 设备初始化状态
  } USB_CDC_Device;
  ```
  
  ## 宏定义
  
  ```c
  #define USB_CDC_RX_DONE_EVENT    (0x01 << 0)  // 接收完成事件
  #define USB_CDC_TX_DONE_EVENT    (0x01 << 1)  // 发送完成事件
  #define CDC_MAX_MPS 64  // 最大包大小
  ```
  
  ## API 接口
  
  ```c
  USB_CDC_Device* cdc_acm_init(uint8_t busid, uintptr_t reg_base, uint16_t expected_rx_len);
  ```
  
  ```c
  void cdc_acm_send(USB_CDC_Device *device, uint8_t* buffer, uint8_t size);
  ```
  
  ```c
  uint8_t* cdc_acm_read(USB_CDC_Device *device);
  ```
  
  ```c
  void cdc_acm_deinit(USB_CDC_Device *device);
  ```
  
  ## 工作原理
  
  ### 初始化流程
  
  1. 调用 cdc_acm_init 初始化 USB CDC ACM 设备
  2. 函数内部初始化设备参数和资源（信号量、事件组等）
  3. 注册设备描述符和端点回调函数
  4. 启动 USB 设备并等待配置完成
  5. 配置完成后启动首次 OUT 端点读取
  
  ### 数据发送流程
  
  1. 应用层调用 cdc_acm_send 发送数据
  2. 函数首先获取发送信号量（确保发送操作的互斥性）
  3. 调用底层 USB 协议栈接口发送数据
  4. 发送完成后，通过回调函数释放信号量
  
  ### 数据接收流程
  
  1. USB 设备初始化时启动首次 OUT 端点读取
  2. 当数据到达时，触发 `usbd_cdc_acm_bulk_out` 回调函数
  3. 回调函数保存接收到的数据长度，设置事件标志，切换活动缓冲区
  4. 应用层调用 cdc_acm_read 等待接收事件
  5. 事件到达后，返回非活动缓冲区的数据指针
  
  ### 双缓冲机制
  
  为了提高数据接收效率，避免数据处理过程中的数据丢失，采用了双缓冲机制：
  
  1. 使用两个缓冲区交替作为活动缓冲区和非活动缓冲区
  2. 当一个缓冲区正在接收数据时，应用层可以处理另一个缓冲区的数据
  3. 通过 rx_active_buf 变量标识当前活动缓冲区
  4. 接收完成时切换活动缓冲区，确保数据的完整性和实时性
  
  ## 中断回调函数
  
  > 注意注意 这里的中断函数由cherryusb接管，将xx_it.c中的void OTG_FS_IRQHandler(void)注释掉，否则编译报错
  
  ```c
  void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes);
  //功能描述： OUT 端点接收完成回调函数，当 USB 设备接收到数据时被调用。
  ```
  
  ```c
  void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes);
  //功能描述： IN 端点发送完成回调函数，当 USB 设备发送数据完成时被调用。
  ```
  
  ## 配置说明
  
  ```c
  //端点配置
  #define CDC_IN_EP  0x81   // IN 端点地址（设备到主机）
  #define CDC_OUT_EP 0x02   // OUT 端点地址（主机到设备）
  #define CDC_INT_EP 0x83   // 中断端点地址 
  //设备描述符配置
  #define USBD_VID           0xFFFF     // 厂商ID
  #define USBD_PID           0xFFFF     // 产品ID
  #define USBD_MAX_POWER     50         // 最大功率（单位：2mA）
  #define USBD_LANGID_STRING 1033       // 语言ID（英语） 
  ```

## 使用示例

```c
#include "cdc_acm.h"
#include "usb_config.h"

// 定义接收缓冲区
static uint8_t user_rx_buffer[2][64];

int main(void)
{
    USB_CDC_Device *cdc_device;

    // 初始化 USB CDC ACM 设备
    cdc_device = cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE, 0);
    if (cdc_device == NULL) {
        // 初始化失败处理
        return -1;
    }

    // 发送数据
    uint8_t send_data[] = "Hello, USB!";
    cdc_acm_send(cdc_device, send_data, sizeof(send_data));

    // 接收数据
    uint8_t *recv_data = cdc_acm_read(cdc_device);
    if (recv_data != NULL) {
        // 处理接收到的数据
        // 注意：recv_data 指向内部缓冲区，处理完后不要释放
    }
    return 0;
}
```

## 注意事项

1. **内存管理**：接收缓冲区内存由驱动内部管理，应用层不需要手动分配和释放
2. **线程安全**：所有 API 都是线程安全的，可以在多个线程中调用
3. **阻塞操作**：cdc_acm_read 函数是事件等待的，会一直等待直到接收到数据
4. **单实例限制**：当前实现只支持单个 CDC 设备实例
5. **缓冲区大小**：接收缓冲区大小受 CDC_MAX_MPS 宏限制

## 错误处理

模块通过以下方式处理错误：

1. **初始化失败**：当设备已初始化或参数错误时，cdc_acm_init 返回 NULL
2. **发送失败**：发送时若参数无效则直接返回，不执行发送操作
3. **接收失败**：设备未初始化或无效时，cdc_acm_read 返回 NULL
4. **反初始化失败**：设备未初始化时，cdc_acm_deinit 直接返回
5. **系统错误**：通过底层 CherryUSB 协议栈处理 USB 相关错误

## 移植说明

要将此驱动移植到其他平台，需要注意以下几点：

1. **寄存器基地址**：根据目标平台修改 reg_base 参数
2. **端点配置**：根据硬件特性调整端点地址和配置
3. **中断处理**：确保正确配置和处理 USB 中断
4. **CherryUSB 配置**：根据需要调整 cherryusb_config.h 中的相关配置
