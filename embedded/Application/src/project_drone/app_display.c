#include "app_display.h"
#include "../app_common.h"
#include "drone_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "e1.h"
#include <stdio.h>

extern SemaphoreHandle_t xDroneDataMutex;
extern DroneData_t g_droneData;

void vTaskDisplay(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[16];
    DroneData_t local;
    uint32_t cnt = 0;

    for (;;) {
        if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_droneData;
            xSemaphoreGive(xDroneDataMutex);
        }

        cnt++;
        /* 数码管轮流显示：高度、温度、档位、飞行时间 */
        if (local.alarm_flag) {
            sprintf(buf, "ALARM");
        } else if (cnt % 20 < 5) {
            sprintf(buf, "H:%.1f", local.altitude);
        } else if (cnt % 20 < 10) {
            sprintf(buf, "T:%.1f", local.temperature);
        } else if (cnt % 20 < 15) {
            sprintf(buf, "G:%d", local.power_gear);
        } else {
            sprintf(buf, "F:%lu", local.flight_time);
        }

        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            e1_tube_str_set(e1_tube_info, buf);
            xSemaphoreGive(xI2CMutex);
        }

        /* LED状态：报警红色、飞行绿色、待命蓝色 */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            if (local.alarm_flag) {
                e1_led_rgb_set(e1_led_info, 100, 0, 0);   // 红
            } else if (local.flight_mode == 1 || local.flight_mode == 2) {
                e1_led_rgb_set(e1_led_info, 0, 100, 0);   // 绿
            } else {
                e1_led_rgb_set(e1_led_info, 0, 0, 100);   // 蓝
            }
            xSemaphoreGive(xI2CMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(150));
    }
}
