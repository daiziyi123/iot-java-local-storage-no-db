#ifndef BSP_LOG_H
#define BSP_LOG_H

#include "FreeRTOS.h"
#include "semphr.h"

// 声明 UART 互斥锁（由 main.c 或 app 层定义，但 BSP 需要 extern）
extern SemaphoreHandle_t xUartMutex;

// 定义加锁打印宏
#define PRINTF_LOCKED(...) do { \
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) { \
        printf(__VA_ARGS__); \
        xSemaphoreGive(xUartMutex); \
    } \
} while(0)

#endif
