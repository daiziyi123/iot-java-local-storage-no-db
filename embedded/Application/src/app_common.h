#ifndef APP_COMMON_H
#define APP_COMMON_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "bsp_log.h"
/* 传感器数据结构（共享状态） */
typedef struct {
    float temperature;
    float humidity;
    float illuminance;
    uint8_t key_state;      // 按键状态，暂未使用
} SensorData_t;

/* 控制命令结构（消息） */
typedef struct {
    uint8_t cmd;            // 'A' ~ 'G'
    int16_t param;          // 保留参数，默认为 -1
} Command_t;

/* 全局互斥锁、队列句柄 */
extern SemaphoreHandle_t xUartMutex;    // 保护串口 printf
extern SemaphoreHandle_t xI2CMutex;     // 保护 I2C 总线（传感器 + 数码管）
extern SemaphoreHandle_t xSensorMutex;  // 保护 SensorData
extern QueueHandle_t xCommandQueue;     // 命令队列

/* 全局传感器数据 */
extern SensorData_t g_sensorData;

#endif
