#include "app_display.h"
#include "../app_common.h"
#include "trash_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "e1.h"
#include <stdio.h>

extern SemaphoreHandle_t xTrashDataMutex;
extern TrashData_t g_trashData;

void vTaskTrashDisplay(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[16];
    TrashData_t local;
    uint32_t cnt = 0;

    for (;;) {
        if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_trashData;
            xSemaphoreGive(xTrashDataMutex);
        }

        cnt++;
        /* 数码管显示内容 */
        if (local.alarm) {
            sprintf(buf, "1");
        } else if (local.compressing) {
            sprintf(buf, "2");
        } else if (local.lid_open) {
            sprintf(buf, "3");
        } else {
            sprintf(buf, "%d%%", local.capacity);
        }

        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            e1_tube_str_set(e1_tube_info, buf);
            xSemaphoreGive(xI2CMutex);
        }

        /* RGB LED 指示 */
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            if (local.alarm) {
                e1_led_rgb_set(e1_led_info, 100, 0, 0);    // 红色：满溢报警
            } else if (local.lid_open) {
                e1_led_rgb_set(e1_led_info, 0, 0, 100);    // 蓝色：盖子打开
            } else if (local.compressing) {
                e1_led_rgb_set(e1_led_info, 100, 100, 0);  // 黄色：压缩中
            } else {
                e1_led_rgb_set(e1_led_info, 0, 100, 0);    // 绿色：正常
            }
            xSemaphoreGive(xI2CMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(150));
    }
}
