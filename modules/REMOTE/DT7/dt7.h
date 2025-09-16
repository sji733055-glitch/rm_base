/*
 * @Author: laladuduqq 2807523947@qq.com
 * @Date: 2025-09-15 09:29:50
 * @LastEditors: laladuduqq 2807523947@qq.com
 * @LastEditTime: 2025-09-15 18:28:46
 * @FilePath: /rm_base/modules/REMOTE/DT7/dt7.h
 * @Description: 
 */
#ifndef _DT7_H_
#define _DT7_H_

#include "bsp_uart.h"
#include "remote_data.h"
#include <stdint.h>

typedef struct {
    int16_t ch1;
    int16_t ch2;
    int16_t ch3;
    int16_t ch4;
    uint8_t sw1;
    uint8_t sw2;
    mouse_state_t mouse_state;
    keyboard_state_t keyboard_state;
    int16_t wheel;
} DT7_INPUT_t;

typedef struct
{
  DT7_INPUT_t dt7_input; 
  uint8_t offline_index; // 离线索引
  UART_Device *uart_device; // UART实例
}DT7_Instance_t;

/**
 * @description: dt7遥控器初始化
 * @param {DT7_Instance_t} *dt7_instance，实例指针
 * @return {osal_status_t}，OSAL_SSCUCCESS初始化成功
 */
osal_status_t dt7_init(DT7_Instance_t *dt7_instance);
/**
 * @description: dt7数据解码
 * @param {DT7_Instance_t} *dt7_instance，实例指针
 * @param {uint8_t} *buf，数据指针
 */
void dt7_decode(DT7_Instance_t *dt7_instance, uint8_t *buf);
/**
 * @description: 获取dt7拨杆状态
 * @param {DT7_Instance_t} *dt7_instance，实例指针
 * @param {uint8_t} sw_index，通道索引
 * @return {enum channel_state}，通道状态
 */
enum channel_state get_dt7_sw_state(DT7_Instance_t *dt7_instance, uint8_t sw_index);

#endif // _DT7_H_