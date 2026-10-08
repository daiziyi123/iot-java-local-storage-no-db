#include "app_sensor.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "s8.h"
#include "s2.h"
#include "s1.h"
#include <stdio.h>


void vTaskSensorRead(void *pvParameters)
{
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        float illuminance = 0;
        s8_ths_t ths = {0};
        uint8_t key = 0;

        /* 读取光照，温湿度，按键（I2C 总线加锁） */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            illuminance= s2_illuminance_value_get(s2_illuminance_info);
            ths = s8_ths_value_get(s8_ths_info);
            key = s1_key_value_get(s1_key_info);
            xSemaphoreGive(xI2CMutex);
        }


        /* 更新共享 SensorData（加锁） */
        if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE) {
            g_sensorData.temperature = ths.temp;
            g_sensorData.humidity    = ths.humi;
            g_sensorData.illuminance = illuminance;
            g_sensorData.key_state   = key;
            xSemaphoreGive(xSensorMutex);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(200));
    }
}

