#include "app_sensor.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "s2.h"
#include "s6.h"
#include "s8.h"
#include <stdio.h>
#include "farm_types.h"
#include <string.h> 

extern SemaphoreHandle_t xFarmDataMutex;
extern FarmData_t g_farmData;

void vTaskFarmSensor(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    float light, temp, humi, dist;

    for (;;) {
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            light = s2_illuminance_value_get(s2_illuminance_info);
            temp  = s8_ths_value_get(s8_ths_info).temp;
            humi  = s8_ths_value_get(s8_ths_info).humi;
            dist  = s6_ultrasonic_distance_get(s6_ultrasonic_info) / 10.0f; // cm
            xSemaphoreGive(xI2CMutex);
        }

        if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
            g_farmData.light        = light;
            g_farmData.temperature  = temp;
            g_farmData.humidity     = humi;
            g_farmData.height       = dist;   // 作物高度近似超声波距离
            xSemaphoreGive(xFarmDataMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}
