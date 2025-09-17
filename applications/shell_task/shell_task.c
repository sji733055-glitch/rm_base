/*
 * @Author: laladuduqq 2807523947@qq.com
 * @Date: 2025-09-17 16:39:19
 * @LastEditors: laladuduqq 2807523947@qq.com
 * @LastEditTime: 2025-09-17 20:38:05
 * @FilePath: /rm_base/applications/shell_task/shell_task.c
 * @Description: 
 */
#include "shell_task.h"
#include "app_config.h"
#include "osal_def.h"
#include "shell.h"
#include <stdint.h>


#ifdef SHELL_MODULE

#define log_tag  "remote_task"
#include "log.h"

static osal_thread_t shell_thread;
SHELL_THREAD_STACK_SECTION static uint8_t shell_thread_stack[SHELL_THREAD_STACK_SIZE];

void shell_task(ULONG thread_input){
    while (1) {
        shell_task_function();
        osal_delay_ms(100);
    }
}

void shell_task_init(){
    osal_status_t status = osal_thread_create(&shell_thread, "shell_task", shell_task, 
        0,shell_thread_stack,SHELL_THREAD_STACK_SIZE,SHELL_THREAD_PRIORITY);
    if (status != OSAL_SUCCESS)
    {
        LOG_ERROR("shell_task init fail");
        return;
    }
    
    osal_thread_start(&shell_thread);
    LOG_INFO("shell_task init success");
     
}

#else 
void shell_task_init(){}
#endif
