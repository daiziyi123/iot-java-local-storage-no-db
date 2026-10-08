#include "app_control.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "farm_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "e1.h"
#include "e2.h"
#include "e3.h"
#include <stdio.h>
#include <stdbool.h>

extern SemaphoreHandle_t xFarmDataMutex;
extern FarmData_t g_farmData;
extern QueueHandle_t xFarmCommandQueue;

#define HUMIDITY_THRESHOLD  30   // 湿度低于30%启动灌溉
#define LIGHT_THRESHOLD     500  // 照度大于500 lux视为强光

static uint8_t manual_override = 0;  // 手动模式标志 (1:手动)

void vTaskFarmControl(void *pvParameters)
{
    char cmd;
    FarmData_t local;
    TickType_t lastSecondTick = xTaskGetTickCount();

    for (;;) {
        /* 处理命令队列 */
        if (xQueueReceive(xFarmCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE) {
            switch (cmd) {
                case 'I': // 远程启动灌溉（开）
                    PRINTF_LOCKED("[Farm] Remote start irrigation.\r\n");
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e2_fan_speed_set(e2_fan_info, 100);  // 水泵/风扇全速
                        xSemaphoreGive(xI2CMutex);
                    }
                    if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_farmData.irrigating = 1;
                        xSemaphoreGive(xFarmDataMutex);
                    }
                    manual_override = 1;   // 进入手动模式
                    break;

                case 'S': // 远程停止灌溉（关）
                    PRINTF_LOCKED("[Farm] Remote stop irrigation.\r\n");
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e2_fan_speed_set(e2_fan_info, 0);
                        xSemaphoreGive(xI2CMutex);
                    }
                    if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_farmData.irrigating = 0;
                        xSemaphoreGive(xFarmDataMutex);
                    }
                    manual_override = 1;
                    break;

                case 'R': // 恢复自动模式
                    PRINTF_LOCKED("[Farm] Restore auto mode.\r\n");
                    manual_override = 0;
                    break;

                
                default:
                    PRINTF_LOCKED("[Farm] Unknown cmd: %c\r\n", cmd);
                    break;
            }
        }

        /* 读取共享数据 */
        if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_farmData;
            xSemaphoreGive(xFarmDataMutex);
        }

        /* ----- 自动灌溉（仅在自动模式下） ----- */
        if (!manual_override) {
            if (local.humidity < HUMIDITY_THRESHOLD && !local.irrigating) {
                // 低湿度启动灌溉
                if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                    e2_fan_speed_set(e2_fan_info, 100);
                    xSemaphoreGive(xI2CMutex);
                }
                if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
                    g_farmData.irrigating = 1;
                    xSemaphoreGive(xFarmDataMutex);
                }
            } else if (local.humidity >= HUMIDITY_THRESHOLD && local.irrigating) {
                // 湿度足够停止灌溉
                if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                    e2_fan_speed_set(e2_fan_info, 0);
                    xSemaphoreGive(xI2CMutex);
                }
                if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
                    g_farmData.irrigating = 0;
                    xSemaphoreGive(xFarmDataMutex);
                }
            }
        }

        /* ----- 自动遮阳（始终自动，不受手动模式影响，可独立） ----- */
        if (local.light > LIGHT_THRESHOLD && !local.shade_closed) {
            // 强光：关闭遮阳棚（位置0表示关闭）
            if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                e3_curtain_position_set(e3_curtain_info, 0);
                xSemaphoreGive(xI2CMutex);
            }
            if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
                g_farmData.shade_closed = 1;
                xSemaphoreGive(xFarmDataMutex);
            }
        } else if (local.light <= LIGHT_THRESHOLD && local.shade_closed) {
            // 弱光：打开遮阳棚
            if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                e3_curtain_position_set(e3_curtain_info, 90);
                xSemaphoreGive(xI2CMutex);
            }
            if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
                g_farmData.shade_closed = 0;
                xSemaphoreGive(xFarmDataMutex);
            }
        }
    }
}
