#include "app_sensor.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "window_types.h"
#include <stdio.h>

/* 假设硬件驱动头文件（实际项目中需替换） */
#include "s1.h"   // 按键
#include "s2.h"   // 照度
#include "s7.h"   // 红外
#include "s8.h"   // 温湿度

extern SemaphoreHandle_t xWindowMutex;
extern windowData_t g_windowData;

void vTaskSensorRead(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    float temp, humi, light;
    unsigned char ir;
  
    for (;;) {
        /* 读取各传感器（I2C总线加锁，若需要） */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            temp = s8_ths_value_get(s8_ths_info).temp;
            humi = s8_ths_value_get(s8_ths_info).humi;
            light = s2_illuminance_value_get(s2_illuminance_info); 
            ir = s7_ir_status_get(s7_ir_info);
            xSemaphoreGive(xI2CMutex);
        }

        /* 更新共享数据（加锁） */
        if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
            g_windowData.temperature = temp;
            g_windowData.humidity = humi;
            g_windowData.light = light;
            g_windowData.pir_detected = ir;
            xSemaphoreGive(xWindowMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}