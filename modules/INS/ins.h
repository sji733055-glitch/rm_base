/*
 * @Author: laladuduqq 2807523947@qq.com
 * @Date: 2025-09-14 08:49:46
 * @LastEditors: laladuduqq 2807523947@qq.com
 * @LastEditTime: 2025-09-17 12:49:41
 * @FilePath: /rm_base/modules/INS/ins.h
 * @Description: INS姿态解算模块
 */

#ifndef _INS_H_
#define _INS_H_

#include "imu_data.h"
#include "imulayer.h"
#include <stdint.h>

// INS核心结构体
typedef struct
{
    // 四元数估计值
    float q[4]; 
    // 机体坐标加速度
    float MotionAccel_b[3]; 
    // 绝对系加速度
    float MotionAccel_n[3]; 
    // 加速度低通滤波系数
    float AccelLPF; 
    // bodyframe在绝对系的向量表示
    float xn[3];
    float yn[3];
    float zn[3];
    // 加速度在机体系和XY两轴的夹角
    // float atanxz;
    // float atanyz;
    // 位姿
    IMU_Estimate_t IMU_Estimate;
    // imu实例
    IMU_Instance_t IMU;
    // 采样时间间隔
    float dt;
    uint32_t dwt_cnt;
} INS_t;

/* 用于修正安装误差的参数 */
typedef struct
{
    uint8_t flag;
    float scale[3];
    float Yaw;
    float Pitch;
    float Roll;
} INS_Param_t;

// 函数声明
void ins_init(void);
void ins_task_function();

#endif // _INS_H_  