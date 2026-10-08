#include "app_display.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include "e1.h"

void vTaskDisplayRefresh(void *pvParameters)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    char disp_buf[16];
    SensorData_t localData;
    uint32_t tick_cnt = 0;

    for (;;) {
        /* 读取共享传感器数据（加锁） */
        if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE) {
            localData = g_sensorData;
            xSemaphoreGive(xSensorMutex);
        }

        /* 轮显：每 2次显示光照，其余显示温度 */
        tick_cnt++;
        if (tick_cnt % 2 == 0)
            sprintf(disp_buf, "L:%.0f", localData.illuminance);
        else
            sprintf(disp_buf, "T:%.1f", localData.temperature);

        /* 更新数码管（I2C 总线加锁） */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            e1_tube_str_set(e1_tube_info, disp_buf);
            xSemaphoreGive(xI2CMutex);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(150));
    }
}


