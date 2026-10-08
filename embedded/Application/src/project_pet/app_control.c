#include "app_control.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "pet_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <stdio.h>
#include "e1.h"
#include "e2.h"
#include "e3.h"
#include <stdbool.h>
/* 自动控制函数（内部） */
extern SemaphoreHandle_t xPetDataMutex;
extern PetData_t g_petData;
extern QueueHandle_t xPetCommandQueue;
static float last_temp = 0.0f;
void vTaskControl(void *pvParameters)
{
    char cmd;
    PetData_t local;
    TickType_t lastSecondTick = xTaskGetTickCount();

    for (;;) {
        /* 处理命令（超时200ms） */
        if (xQueueReceive(xPetCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE) {
            switch (cmd) {
                case 'H':
                    PRINTF_LOCKED("[Pet] Remote feed.\r\n");
                    e3_curtain_position_set(e3_curtain_info, 90);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    e3_curtain_position_set(e3_curtain_info, 0);
                    if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_petData.feed_reminder = 0;
                        xSemaphoreGive(xPetDataMutex);
                    }
                    break;
                case 'Q':
                    PRINTF_LOCKED("[Pet] Query.\r\n");
                    PRINTF_LOCKED("{\"project\": 13,\"name\":\"Pet House\",\"distance\":%.1f,\"temperature\":%.1f,\"humidity\":%.1f,\"in_nest\":%d,\"stay_seconds\":%lu,\"feed_reminder\":%d}",
    local.distance,local.temperature, local.humidity, local.in_nest, local.stay_seconds, local.feed_reminder);
                    break;
                default:
                    PRINTF_LOCKED("[Pet] Unknown: %c\r\n", cmd);
                    break;
            }
        }

        /* 自动控制 */
        if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_petData;
            xSemaphoreGive(xPetDataMutex);
        }

        /* 温控 */
        if (local.temperature < 30.0f) {
        if (last_temp >= 30.0f)      // 仅当状态变化时设置
        e2_fan_speed_set(e2_fan_info, 100);
        } else {
        if (last_temp < 30.0f)
        e2_fan_speed_set(e2_fan_info, 0);
        }
        last_temp = local.temperature;

        /* 投食提醒 */
        if (local.pir_detected && local.in_nest) {
            if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
                g_petData.feed_reminder = 1;
                xSemaphoreGive(xPetDataMutex);
            }
        }

        /* 时长累计 */
        TickType_t now = xTaskGetTickCount();
        if (local.in_nest) {
            if ((now - lastSecondTick) >= pdMS_TO_TICKS(1000)) {
                lastSecondTick = now;
                if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
                    g_petData.stay_seconds++;
                    xSemaphoreGive(xPetDataMutex);
                }
            }
        } else {
            if (g_petData.stay_seconds != 0) {
                if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE) {
                    g_petData.stay_seconds = 0;
                    xSemaphoreGive(xPetDataMutex);
                }
            }
            lastSecondTick = now;
        }
    }
}