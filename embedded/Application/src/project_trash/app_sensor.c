#include "app_sensor.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "s6.h"
#include "s7.h"
#include <stdio.h>
#include "trash_types.h"

extern SemaphoreHandle_t xTrashDataMutex;
extern TrashData_t g_trashData;

void vTaskTrashSensor(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    float dist;
    unsigned char  ir;

    for (;;) {
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            dist = s6_ultrasonic_distance_get(s6_ultrasonic_info) / 10.0f; // 单位 cm
            ir   = s7_ir_status_get(s7_ir_info);
            xSemaphoreGive(xI2CMutex);
        }

        if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
            g_trashData.distance = dist;
            g_trashData.person_detected = ir;
            // 容量换算：假设最大距离 100 cm，容量 = (100 - dist) / 100 * 100，限制 0~100
            if (dist < 0) dist = 0;
            if (dist > 100) dist = 100;
            uint8_t cap = (uint8_t)((100.0f - dist) / 100.0f * 100.0f);
            if (cap > 100) cap = 100;
            g_trashData.capacity = cap;

            xSemaphoreGive(xTrashDataMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}