/*
 * @Author: laladuduqq 2807523947@qq.com
 * @Date: 2025-09-15 09:18:31
 * @LastEditors: laladuduqq 2807523947@qq.com
 * @LastEditTime: 2025-09-16 08:24:01
 * @FilePath: /rm_base/modules/REMOTE/remote.h
 * @Description: 
 */
#ifndef _REMOTE_H_
#define _REMOTE_H_


#include <stdint.h>
#include "dt7.h"
#include "modules_config.h"
#include "sbus.h"
#include "vt02.h"
#include "vt03.h"
#include "osal_def.h"

typedef struct {
#if defined (REMOTE_SOURCE) && REMOTE_SOURCE == 1
    SBUS_Instance_t sbus_instance;    
#elif defined (REMOTE_SOURCE) && REMOTE_SOURCE == 2
    DT7_Instance_t dt7_instance;
#endif
#if defined (REMOTE_VT_SOURCE) && REMOTE_VT_SOURCE == 1
    VT02_Instance_t vt02_instance; 
#elif defined (REMOTE_VT_SOURCE) && REMOTE_VT_SOURCE == 2
    VT03_Instance_t vt03_instance;
#endif
    uint8_t vt_offline_index;
    uint8_t remote_offline_index;
    uint8_t initflag;
} remote_instance_t;

/**
 * @description: 遥控器初始化
 * @return {osal_status_t}，初始化结果
 */
osal_status_t remote_init(void);
/**
 * @brief 获取遥控器通道状态
 * @param remote_instance 遥控器实例指针
 * @param channel_index 通道索引
 * @param is_vt_remote 是否为图传遥控器
 * @return uint8_t 通道状态
 */
uint8_t get_remote_channel_state(remote_instance_t *remote_instance, uint8_t channel_index, uint8_t is_vt_remote);
/**
 * @brief 获取鼠标状态
 * @param remote_instance 遥控器实例指针
 * @param is_vt_remote 是否为图传遥控器
 * @return mouse_state_t* 鼠标状态指针
 */
mouse_state_t* get_remote_mouse(remote_instance_t *remote_instance, uint8_t is_vt_remote);
/**
 * @brief 获取键盘按键状态
 * @param remote_instance 遥控器实例指针
 * @param is_vt_remote 是否为图传遥控器
 * @return keyboard_state_t* 键盘状态指针
 */
keyboard_state_t * get_remote_keyboard_state(remote_instance_t *remote_instance,uint8_t is_vt_remote);
/**
 * @brief 遥控器任务函数
 */
void remote_task_function(void);
/**
 * @brief 图传遥控器任务函数
 */
void remote_vt_task_function(void);
/**
 * @brief 获取遥控器实例指针
 * @return remote_instance_t* 遥控器实例指针
 */
remote_instance_t* get_remote_instance(void);

#endif // _REMOTE_H_