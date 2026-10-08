#include "app_control.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "window_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <stdio.h>
#include <stdbool.h>

/* 执行器驱动（假设） */
#include "e1.h"   // LED+数码管
#include "e2.h"   // 风扇
#include "e3.h"   // 窗帘

extern SemaphoreHandle_t xWindowMutex;
extern windowData_t g_windowData;
extern QueueHandle_t xWindowCommandQueue;



void vTaskControl(void *pvParameters)
{
    char cmd;
    windowData_t local;
    TickType_t lastSecondTick = xTaskGetTickCount();

    for (;;) {
        /* 处理命令（超时200ms） */
        if (xQueueReceive(xWindowCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE) {
            switch (cmd) {
                case 'C':  // 远程关窗
                    if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                        g_windowData.curtain_pos = 0;
                        g_windowData.opening = 0;
                        g_windowData.ifauto = 0;
                        xSemaphoreGive(xWindowMutex);
                        PRINTF_LOCKED("WINDOW CLOSE\r\n");
                    }
                    e3_curtain_position_set(e3_curtain_info, 0);
                    break;
                case 'O':  // 远程开窗
                    if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                        g_windowData.curtain_pos = 100;
                        g_windowData.opening = 1;
                        g_windowData.ifauto = 0;
                        xSemaphoreGive(xWindowMutex);
                        PRINTF_LOCKED("WINDOW OPEM\r\n");
                    }
                    e3_curtain_position_set(e3_curtain_info, 100);
                    break;
                case 'A':  // 切换为自动模式（远程）
                    if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                        g_windowData.ifauto=1;
                        xSemaphoreGive(xWindowMutex);
                        PRINTF_LOCKED("AUTO STYLE\r\n");
                    }
                    break;
                default:
                    PRINTF_LOCKED("[Window] Unknown cmd: %c\r\n", cmd);
                    break;
            }
        }

        /* 读取最新共享数据 */
        if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
            local = g_windowData;
            xSemaphoreGive(xWindowMutex);
        }
        

        /* ---- 自动控制逻辑 ---- */

        // 1. 无人检测：若无人且是自动调控的情况下，关闭窗户和灯光（强制）
        if (!local.pir_detected && local.ifauto) {
            // 关闭窗户（合上）
            if (local.curtain_pos != 0) {
                if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                    g_windowData.curtain_pos = 0;
                    g_windowData.opening = 0;
                  
                    xSemaphoreGive(xWindowMutex);
                }
                e3_curtain_position_set(e3_curtain_info, 0);
            }
            // 无人时，不累计占用时间，但总时间仍累计
        } 
        else if (local.ifauto)
        {     // 自动模式且有人
        uint8_t should_close = 0;

            // 检查关窗条件
            if (local.humidity >= 80) {
               should_close = 1;
            }
            if (local.light >= 1000) {
               should_close = 1;
            }

            if (should_close) {
            // 关窗
                if (local.curtain_pos != 0) {
                  if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                    g_windowData.curtain_pos = 0;
                    g_windowData.opening = 0;
                    xSemaphoreGive(xWindowMutex);
                }
            e3_curtain_position_set(e3_curtain_info, 0);
        }
            }else {
             // 所有条件均不满足且有人 → 开窗
                if (local.curtain_pos != 100) {
                  if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                    g_windowData.curtain_pos = 100;
                  g_windowData.opening = 1;
                  xSemaphoreGive(xWindowMutex);
                }
                e3_curtain_position_set(e3_curtain_info, 100);
             }
            }
            
        } 

        /* ---- 能耗统计（每秒累计） ---- */
        TickType_t now = xTaskGetTickCount();
        if ((now - lastSecondTick) >= pdMS_TO_TICKS(1000)) {
            lastSecondTick = now;
            if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                g_windowData.total_sec++;
                if (local.pir_detected) {
                    g_windowData.occupancy_sec++;
                }
                if (local.curtain_pos > 0) {
                    g_windowData.curtain_runtime++;
                }
                xSemaphoreGive(xWindowMutex);
            }
        }
    }
}