# IMU 模块文档

## 概述

IMU（Inertial Measurement Unit，惯性测量单元）模块提供了一个统一的接口来操作各种类型的惯性测量传感器。该模块采用了兼容层设计，允许用户在不同的IMU硬件之间轻松切换，而无需修改上层应用代码。

当前支持的IMU类型包括BMI088，未来可以轻松扩展以支持其他类型的IMU传感器，如ICM42688等。

## 特性

- 统一的API接口，屏蔽底层硬件差异
- 支持多种IMU传感器类型
- 内置温度控制功能
- 自动校准功能
- 支持条件编译，仅编译使用的传感器代码

## 数据结构

### IMU_Data_t

```c
typedef struct {
    float gyro[3];     // 陀螺仪数据，xyz轴角速度（单位：rad/s）
    float acc[3];      // 加速度计数据，xyz轴加速度（单位：m/s²）
    float temperature; // 温度（单位：摄氏度）
} IMU_Data_t;
```

### IMU_Instance_t

```c
typedef struct {
    uint8_t init_flag;     // 初始化标志，0-未初始化，1-已初始化
    IMU_Data_t data;       // IMU数据
#if IMU_TYPE == IMU_BMI088
    BMI088_Instance_t imu_handle;  // BMI088实例句柄
#endif
} IMU_Instance_t;
```

## API 接口

```c
osal_status_t imu_init(IMU_Instance_t *ist);
```

```c
osal_status_t imu_get_accel(IMU_Instance_t *ist);
```

```c
osal_status_t imu_get_gyro(IMU_Instance_t *ist);
```

```c
osal_status_t imu_get_temp(IMU_Instance_t *ist);
```

```c
void imu_temp_ctrl(IMU_Instance_t *ist, float temp);
```

## 配置说明

```c
/* IMU 模块 */
#define IMU_BMI088   1
#define IMU_ICM42688 2
#define IMU_TYPE     IMU_BMI088     // 选择要使用的IMU类型

#if IMU_TYPE == IMU_BMI088
   #define BMI088_TEMP_ENABLE 1     // 启用BMI088模块的温度控制
   #define BMI088_TEMP_SET    35.0f // BMI088的目标温度
#elif IMU_TYPE == IMU_ICM42688
   // ICM42688相关配置
#endif
```

## 使用示例

```c
#include "imulayer.h"

// 定义IMU实例
IMU_Instance_t imu_instance;

int main(void)
{
    // 初始化IMU
    if (imu_init(&imu_instance) != OSAL_SUCCESS) {
        // 初始化失败处理
        return -1;
    }
    
    while (1) {
        // 获取加速度数据
        if (imu_get_accel(&imu_instance) == OSAL_SUCCESS) {
            float ax = imu_instance.data.acc[0];
            float ay = imu_instance.data.acc[1];
            float az = imu_instance.data.acc[2];
            // 处理加速度数据
        }
        
        // 获取陀螺仪数据
        if (imu_get_gyro(&imu_instance) == OSAL_SUCCESS) {
            float gx = imu_instance.data.gyro[0];
            float gy = imu_instance.data.gyro[1];
            float gz = imu_instance.data.gyro[2];
            // 处理陀螺仪数据
        }
        
        // 获取温度数据
        if (imu_get_temp(&imu_instance) == OSAL_SUCCESS) {
            float temp = imu_instance.data.temperature;
            // 温度控制
            imu_temp_ctrl(&imu_instance, temp);
        }
        
        // 延时或其他操作
        osal_thread_sleep(10); // 10ms延时
    }
}
```

## 添加新IMU支持的示例

### 1. 修改配置文件

在CONFIG/modules_config.h中添加新IMU的定义：

```c
/* IMU 模块 */
#define IMU_BMI088   1
#define IMU_ICM42688 2
#define NEW_IMU      3
#define IMU_TYPE     NEW_IMU     // 选择新的IMU类型

#if IMU_TYPE == IMU_BMI088
   #define BMI088_TEMP_ENABLE 1     
   #define BMI088_TEMP_SET    35.0f 
#elif IMU_TYPE == NEW_IMU
   #define TEMP_ENABLE 1
   #define TEMP_SET    40.0f
#endif
```

### 2. 创建新IMU驱动文件

创建`modules/IMU/NEW_IMU/`目录，并在其中添加对应的文件：

```c
#ifndef _ICM42688_H_
#define _ICM42688_H_

#include "imu_data.h"
#include "bsp_spi.h"
#include "osal_def.h"

typedef struct {
    uint8_t buf[16];
    SPI_Device *device;
    // 其他特定于ICM42688的字段
} ICM42688_Instance_t;

osal_status_t ICM42688_init(ICM42688_Instance_t *ist);
osal_status_t icm42688_get_accel(ICM42688_Instance_t *ist, IMU_Data_t *data);
osal_status_t icm42688_get_gyro(ICM42688_Instance_t *ist, IMU_Data_t *data);
osal_status_t icm42688_get_temp(ICM42688_Instance_t *ist, IMU_Data_t *data);
void icm42688_temp_ctrl(ICM42688_Instance_t *ist, float temp);

#endif // _ICM42688_H_
```

### 3. 修改IMU兼容层

更新modules/IMU/IMULAYER/imulayer.h文件：

```c
// ... existing code ...

#include "bmi088.h"
#include "icm42688.h"  // 添加新IMU头文件
#include "imu_data.h"
#include "osal_def.h"
#include <stdint.h>

typedef struct {
    uint8_t init_flag;
    IMU_Data_t data;
#if IMU_TYPE == IMU_BMI088
    BMI088_Instance_t imu_handle;
#elif IMU_TYPE == IMU_ICM42688
    ICM42688_Instance_t imu_handle;  // 添加新IMU实例
#endif
} IMU_Instance_t;

// ... existing code ...
```

更新`modules/IMU/IMULAYER/imulayer.c`文件：

```c
// ... existing code ...

#if IMU_TYPE == IMU_BMI088
#include "BMI088/bmi088.h"
#elif IMU_TYPE == IMU_ICM42688
#include "ICM42688/icm42688.h"  // 添加新IMU头文件
#endif

osal_status_t imu_init(IMU_Instance_t *ist)
{
    if (ist == NULL)
    {
        return OSAL_ERROR;
    }

    osal_status_t status = OSAL_SUCCESS;
    
#if IMU_TYPE == IMU_BMI088
    status = BMI088_init(&ist->imu_handle);
    if (status == OSAL_SUCCESS) {
        ist->init_flag = 1;
    }
#elif IMU_TYPE == IMU_ICM42688
    status = ICM42688_init(&ist->imu_handle);
    if (status == OSAL_SUCCESS) {
        ist->init_flag = 1;
    }
#endif

    return status;
}

// ... 对其他函数做类似修改 ...
```

完成以上步骤后，就可以通过修改modules_config.h中的IMU_TYPE定义来在不同IMU之间切换，而无需修改上层应用代码。

## 注意事项

1. **温度控制**：启用温度控制功能时，确保硬件支持加热器电路。

2. **校准**：首次使用或更换传感器时，建议执行校准程序以获得最佳精度。

3. **条件编译**：使用条件编译确保只编译实际使用的IMU代码，减小程序体积。


