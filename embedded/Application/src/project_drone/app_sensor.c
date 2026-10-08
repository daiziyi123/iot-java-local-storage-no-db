#include "app_sensor.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "s1.h"      // 按键
#include "s6.h"      // 超声波
#include "s8.h"      // 温湿度
#include "drone_types.h"
#include <stdio.h>
extern SemaphoreHandle_t xDroneDataMutex;
extern DroneData_t g_droneData;

void vTaskSensorRead(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    float alt, temp, humi;
    uint8_t key_press;

    for (;;) {
        /* 读取传感器（I2C总线加锁） */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            alt  = s6_ultrasonic_distance_get(s6_ultrasonic_info) / 10.0f; // cm
            temp = s8_ths_value_get(s8_ths_info).temp;
            humi = s8_ths_value_get(s8_ths_info).humi;
            key_press = s1_key_value_get(s1_key_info); // 假设按键状态
            xSemaphoreGive(xI2CMutex);
        }

        /* 更新共享数据 */
        if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
            g_droneData.altitude    = alt;
            g_droneData.temperature = temp;
            g_droneData.humidity    = humi;
            /* 按键调节档位：短按切换 0->1->2->0 */
            if (key_press) {
                g_droneData.power_gear = (g_droneData.power_gear + 1) % 3;
                PRINTF_LOCKED("[Drone] Gear changed to %d\r\n", g_droneData.power_gear);
            }
            xSemaphoreGive(xDroneDataMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}