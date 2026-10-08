#include "app_sensor.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "classroom_types.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>

/* 假设硬件驱动头文件（实际项目中需替换） */
#include "s1.h" // 按键
#include "s2.h" // 照度
#include "s7.h" // 红外
#include "s8.h" // 温湿度

extern SemaphoreHandle_t xClassroomMutex;
extern ClassroomData_t g_classroomData;

void vTaskSensorRead(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    float temp, humi, light;
    uint8_t key;
    unsigned char ir;
    static uint8_t last_key = 0;

    for (;;)
    {
        /* 读取各传感器（I2C总线加锁，若需要） */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE)
        {
            temp = s8_ths_value_get(s8_ths_info).temp;
            humi = s8_ths_value_get(s8_ths_info).humi;
            light = s2_illuminance_value_get(s2_illuminance_info); // 假设照度传感器.接口
            ir = s7_ir_status_get(s7_ir_info);
            key = s1_key_value_get(s1_key_info); // 按键读取（0/1）
            xSemaphoreGive(xI2CMutex);
        }

        /* 更新共享数据（加锁） */
        if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
        {
            g_classroomData.temperature = temp;
            g_classroomData.humidity = humi;
            g_classroomData.light = light;
            g_classroomData.pir_detected = ir;
            // 在读取后：
            if (key && !last_key)
            {
                g_classroomData.key_pressed = 1;
            }
            last_key = key;
            xSemaphoreGive(xClassroomMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}
