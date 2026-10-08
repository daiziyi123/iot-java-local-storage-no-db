#include "app_display.h"
#include "../app_common.h"
#include "farm_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "e1.h"
#include <stdio.h>

extern SemaphoreHandle_t xFarmDataMutex;
extern FarmData_t g_farmData;

void vTaskFarmDisplay(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[16];
    FarmData_t local;
    uint32_t cnt = 0;

    for (;;) {
        if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_farmData;
            xSemaphoreGive(xFarmDataMutex);
        }

        cnt++;
        /* 数码管显示：交替显示湿度或高度 */
        if (cnt % 10 < 5) {
            sprintf(buf, "H:%d", (int)(local.humidity));
        } else {
            sprintf(buf, "Ht:%.1f", local.height);
        }

        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            e1_tube_str_set(e1_tube_info, buf);
            xSemaphoreGive(xI2CMutex);
        }

        /* RGB LED 指示土壤湿度状态 */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            if (local.humidity < 30) {
                e1_led_rgb_set(e1_led_info, 100, 0, 0);   // 红色：干旱
            } else if (local.humidity < 60) {
                e1_led_rgb_set(e1_led_info, 100, 100, 0); // 黄色：适中
            } else {
                e1_led_rgb_set(e1_led_info, 0, 100, 0);   // 绿色：湿润
            }
            xSemaphoreGive(xI2CMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(150));
    }
}