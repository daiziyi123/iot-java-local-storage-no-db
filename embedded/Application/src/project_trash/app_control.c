#include "app_control.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "trash_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "e1.h"
#include "e2.h"
#include "e3.h"
#include <stdio.h>
#include <stdbool.h>

extern SemaphoreHandle_t xTrashDataMutex;
extern TrashData_t g_trashData;
extern QueueHandle_t xTrashCommandQueue;

static uint32_t compress_tick = 0;   // 压缩计时（秒）
static uint8_t  compress_phase = 0;  // 0:停止, 1:运行
static uint8_t  manual_override = 0;
void vTaskTrashControl(void *pvParameters)
{
    char cmd;
    TrashData_t local;
    TickType_t lastSecondTick = xTaskGetTickCount();

    for (;;) {
        /* 处理命令队列（超时 200ms） */
        if (xQueueReceive(xTrashCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE) {
            switch (cmd) {
                case 'O': // 远程开盖
                    PRINTF_LOCKED("[Trash] Remote open lid.\r\n");
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e3_curtain_position_set(e3_curtain_info, 90);
                        xSemaphoreGive(xI2CMutex);
                    }
                    if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_trashData.lid_open = 1;
                        xSemaphoreGive(xTrashDataMutex);
                    }
                    manual_override = 1; 
                    break;

                case 'C': // 远程关盖
                    PRINTF_LOCKED("[Trash] Remote close lid.\r\n");
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e3_curtain_position_set(e3_curtain_info, 0);
                        xSemaphoreGive(xI2CMutex);
                    }
                    if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_trashData.lid_open = 0;
                        xSemaphoreGive(xTrashDataMutex);
                    }
                    manual_override = 1; 
                    break;
                case 'R': 
                    PRINTF_LOCKED("[Trash] Restore auto mode.\r\n");
                    manual_override = 0;
                    break;
                default:
                    PRINTF_LOCKED("[Trash] Unknown cmd: %c\r\n", cmd);
                    break;
            }
        }

        /* 读取共享数据 */
        if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_trashData;
            xSemaphoreGive(xTrashDataMutex);
        }

        /* ----- 自动开/关盖（有人开，无人关） ----- */
        if (!manual_override) { 
        if (local.person_detected && !local.lid_open) {
            if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                e3_curtain_position_set(e3_curtain_info, 90);
                xSemaphoreGive(xI2CMutex);
            }
            if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                g_trashData.lid_open = 1;
                xSemaphoreGive(xTrashDataMutex);
            }
        } else if (!local.person_detected && local.lid_open) {
            if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                e3_curtain_position_set(e3_curtain_info, 0);
                xSemaphoreGive(xI2CMutex);
            }
            if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                g_trashData.lid_open = 0;
                xSemaphoreGive(xTrashDataMutex);
            }
        }
    }
        /* ----- 满溢报警（容量 > 80% 置位 alarm） ----- */
        if (local.capacity > 80) {
            if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                g_trashData.alarm = 1;
                xSemaphoreGive(xTrashDataMutex);
            }
        } else {
            if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                g_trashData.alarm = 0;
                xSemaphoreGive(xTrashDataMutex);
            }
        }

        /* ----- 模拟压缩：风扇间歇运行（每10秒运行3秒） ----- */
        TickType_t now = xTaskGetTickCount();
        if ((now - lastSecondTick) >= pdMS_TO_TICKS(1000)) {
            lastSecondTick = now;
            compress_tick++;

            if (compress_tick % 10 < 3) { // 前3秒运行
                if (!compress_phase) {
                    compress_phase = 1;
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e2_fan_speed_set(e2_fan_info, 80); // 中速
                        xSemaphoreGive(xI2CMutex);
                    }
                    if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_trashData.compressing = 1;
                        xSemaphoreGive(xTrashDataMutex);
                    }
                }
            } else { // 后7秒停止
                if (compress_phase) {
                    compress_phase = 0;
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e2_fan_speed_set(e2_fan_info, 0);
                        xSemaphoreGive(xI2CMutex);
                    }
                    if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_trashData.compressing = 0;
                        xSemaphoreGive(xTrashDataMutex);
                    }
                }
            }
        }
    }
}
