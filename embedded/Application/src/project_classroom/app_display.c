#include "app_display.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "classroom_types.h"
#include "e1.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>

extern SemaphoreHandle_t xClassroomMutex;
extern ClassroomData_t g_classroomData;

void vTaskDisplay(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[16];
    ClassroomData_t local;
    uint32_t cnt = 0;

    for (;;)
    {
        if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
        {
            local = g_classroomData;
            xSemaphoreGive(xClassroomMutex);
        }

        cnt++;
        // 数码管轮显：模式、温度、照度、使用率
        if (cnt % 4 == 0)
        {
            sprintf(buf, "Md%d", local.mode);
        }
        else if (cnt % 4 == 1)
        {
            sprintf(buf, "T%.1f", local.temperature);
        }
        else if (cnt % 4 == 2)
        {
            sprintf(buf, "L%.0f", local.light);
        }
        else
        {
            uint32_t usage = (local.total_sec > 0) ? (local.occupancy_sec * 100 / local.total_sec) : 0;
            sprintf(buf, "U%lu", usage);
        }

        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE)
        {
            e1_tube_str_set(e1_tube_info, buf);
            xSemaphoreGive(xI2CMutex);
        }

        // LED颜色指示模式：正常-绿，节能-蓝，演示-黄
        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE)
        {
            if (!g_classroomData.manual_lock)
            {
                switch (local.mode)
                {
                case 0:
                    e1_led_rgb_set(e1_led_info, 0, 100, 0);
                    break; // 绿
                case 1:
                    e1_led_rgb_set(e1_led_info, 0, 0, 100);
                    break; // 蓝
                case 2:
                    e1_led_rgb_set(e1_led_info, 100, 100, 0);
                    break; // 黄
                default:
                    e1_led_rgb_set(e1_led_info, 100, 0, 0);
                    break;
                }
            }

            xSemaphoreGive(xI2CMutex);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(300));
    }
}
