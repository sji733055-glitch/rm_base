/*
 * @Author: laladuduqq 2807523947@qq.com
 * @Date: 2025-09-15 23:49:52
 * @LastEditors: laladuduqq 2807523947@qq.com
 * @LastEditTime: 2025-10-01 19:22:41
 * @FilePath: /rm_base/applications/remote_task/remote_task.h
 * @Description: 
 */
#ifndef _REMOTE_TASK_H_
#define _REMOTE_TASK_H_

#include "remote.h"


void remote_task_init();
/**
 * @description: 获取遥控器实例指针
 * @return {remote_instance_t*}, 遥控器实例指针
 */
remote_instance_t *get_remote_instance();

#endif // _REMOTE_TASK_H_