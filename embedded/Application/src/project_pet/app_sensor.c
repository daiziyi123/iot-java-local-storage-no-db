#include "app_sensor.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "s6.h"
#include "s7.h"
#include "s8.h"
#include <stdio.h>
#include "pet_types.h"
extern SemaphoreHandle_t xPetDataMutex;
extern PetData_t g_petData;

void vTaskSensorRead(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    float dist, temp, humi;
    uint8_t ir;

    for (;;) {
        /* 读取各传感器（I2C总线加锁） */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            temp = s8_ths_value_get(s8_ths_info).temp;
            dist = s6_ultrasonic_distance_get(s6_ultrasonic_info)/10;
            humi = s8_ths_value_get(s8_ths_info).humi;
            ir  = s7_ir_status_get(s7_ir_info);
            xSemaphoreGive(xI2CMutex);
        }
        
        /* 更新共享数据（加锁） */
        if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
            g_petData.distance      = dist;
            g_petData.temperature   = temp;
            g_petData.humidity      = humi;
            g_petData.pir_detected  = ir;
            g_petData.in_nest       = (dist < 40.0f) ? 1 : 0;
            xSemaphoreGive(xPetDataMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}