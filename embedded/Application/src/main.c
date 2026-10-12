#include "gd32f4xx.h"
#include "delay.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "app_common.h"
#include "app_hardware.h"
#include <stdio.h>
/* 外设驱动头文件（用于初始化） */
#include "e1.h"
#include "c3.h"
#include "s8.h"
#include "s2.h"
#include "e3.h"
#include "e2.h"
#include "s1.h"
#include "s6.h"
#include "s7.h"
#include "i2c.h"
#define PROJECT_ID 20

#if PROJECT_ID == 1
    #include "Project1/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks
#elif PROJECT_ID == 13
    #include "Project_Pet/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_pet
#elif PROJECT_ID == 20
    #include "Project_classroom/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_classroom
#elif PROJECT_ID == 15
    #include "Project_window/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_window
#elif PROJECT_ID == 25
    #include "Project_drone/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_drone
#elif PROJECT_ID == 16
    #include "Project_trash/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_trash
#elif PROJECT_ID == 17
    #include "Project_farm/app_project.h"
    #define APP_CREATE_TASKS app_project_create_tasks_farm
#else
    #error "Unknown PROJECT_ID"
#endif

/* 全局互斥锁（所有项目共享） */
SemaphoreHandle_t xUartMutex;
SemaphoreHandle_t xI2CMutex;

int i2c_reg_bytes_read(i2c_slave_info info, unsigned char reg, unsigned char * pbytes, unsigned char count);
static void BSP_Init(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    timer3_init();
    uart_init(USART0);
    uart_init(USART5);
    uart_init(USART1);
/* 初始化所有外设（未使用的项目不会调用，但初始化无害） */
    i2c_init();
    e1_tube_info = e1_tube_init();
    e1_led_info  = e1_led_init();
    e2_fan_info  = e2_fan_init();
    e3_curtain_info = e3_curtain_init();
    s8_ths_info  = s8_ths_init();
    s2_illuminance_info = s2_illuminance_init();
    s1_key_info  = s1_key_init();
    s6_ultrasonic_info = s6_ultrasonic_init();
    s7_ir_info  = s7_ir_init();
}

/* ---------- printf 重定向（带 UART 互斥） ---------- */
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
FILE __stdout;
void _sys_exit(int x) { x = x; }
void _ttywrch(int ch) { ch = ch; }
int fputc(int ch, FILE *f)
{
    // 不再在此处加锁，假设调用者已经持有 xUartMutex
    usart_data_transmit(USART1, ch);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
    return ch;
}

static void AppTaskCreate(void)
{
    xUartMutex = xSemaphoreCreateMutex();
    xI2CMutex  = xSemaphoreCreateMutex();
    APP_CREATE_TASKS();  // 调用当前项目的创建函数
}

int main(void)
{
    BSP_Init();
    AppTaskCreate();
    vTaskStartScheduler();
    while (1);
}
