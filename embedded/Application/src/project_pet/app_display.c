#include "app_display.h"
#include "../app_common.h"
#include "pet_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "e1.h"
#include <stdio.h>

extern SemaphoreHandle_t xPetDataMutex;
extern PetData_t g_petData;

void vTaskDisplay(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[16];
    PetData_t local;
    uint32_t cnt = 0;

    for (;;) {
        if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_petData;
            xSemaphoreGive(xPetDataMutex);
        }

        cnt++;
        if (local.feed_reminder) {
            sprintf(buf, "Fd");
        } else if (cnt % 15 < 5) {
            sprintf(buf, "N:%d", local.in_nest);
        } else if (cnt % 15 < 10) {
            sprintf(buf, "T:%.1f", local.temperature);
        } else {
            sprintf(buf, "S:%lu", local.stay_seconds);
        }

        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            e1_tube_str_set(e1_tube_info, buf);
            xSemaphoreGive(xI2CMutex);
        }

        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
            if (local.feed_reminder)
            e1_led_rgb_set(e1_led_info, 100, 100, 0);
            else if (local.in_nest)
            e1_led_rgb_set(e1_led_info, 0, 100, 0);
            else
            e1_led_rgb_set(e1_led_info, 100, 0, 0);
        xSemaphoreGive(xI2CMutex);
}
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}