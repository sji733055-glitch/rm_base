/*
 * @Author: laladuduqq 2807523947@qq.com
 * @Date: 2025-09-15 09:30:15
 * @LastEditors: laladuduqq 2807523947@qq.com
 * @LastEditTime: 2025-09-15 18:31:09
 * @FilePath: /rm_base/modules/REMOTE/VT03/vt03.h
 * @Description: 
 */
#ifndef _VT03_H_
#define _VT03_H_

/* 按键位定义 */
#include "bsp_uart.h"
#include "remote_data.h"
#include <stdint.h>


typedef struct {
    int16_t ch1;     
    int16_t ch2;     
    int16_t ch3;     
    int16_t ch4;    
    int16_t wheel;        
    button_state_t button_state;
    mouse_state_t mouse_state;
    keyboard_state_t key_state;
}vt03_remote_data_t;


typedef struct{
    vt03_remote_data_t vt03_remote_data;
    uint8_t offline_index; // 离线索引
    UART_Device *uart_device; // UART实例
}VT03_Instance_t;

/**
 * @description: VT03图传初始化
 * @param {VT03_Instance_t} *vt03_instance
 * @return {osal_status_t}，OSAL_SCUCCESS表示成功
 */
osal_status_t vt03_init(VT03_Instance_t *vt03_instance);
/**
 * @description: VT03图传解码
 * @param {VT03_Instance_t} *vt03_instance
 * @param {uint8_t} *buf
 * @return {void}
 */
void vt03_decode(VT03_Instance_t *vt03_instance, uint8_t *buf);

#endif // _VT03_H_