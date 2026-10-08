#include "app_control.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "drone_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "e1.h"
#include "e2.h"
#include "e3.h"
#include <stdio.h>
#include <stdbool.h>

extern SemaphoreHandle_t xDroneDataMutex;
extern DroneData_t g_droneData;
extern QueueHandle_t xDroneCommandQueue;

static float last_temp = 0.0f;
static float last_humi = 0.0f;

void vTaskControl(void *pvParameters)
{
    char cmd;
    DroneData_t local;
    TickType_t lastSecondTick = xTaskGetTickCount();

    for (;;) {
        /* 处理命令队列（超时200ms） */
        if (xQueueReceive(xDroneCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE) {
            switch (cmd) {
                case 'T':  // 起飞 (Takeoff)
                    PRINTF_LOCKED("[Drone] Command: Takeoff\r\n");
                    if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_droneData.flight_mode = 1;
                        xSemaphoreGive(xDroneDataMutex);
                    }
                    // 模拟动力：开启风扇（E2）高速
                    e2_fan_speed_set(e2_fan_info, 80);
                    break;
                case 'L':  // 降落 (Land)
                    PRINTF_LOCKED("[Drone] Command: Land\r\n");
                    if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_droneData.flight_mode = 3;
                        xSemaphoreGive(xDroneDataMutex);
                    }
                    e2_fan_speed_set(e2_fan_info, 0);
                    break;
                case 'H':  // 悬停 (Hover)
                    PRINTF_LOCKED("[Drone] Command: Hover\r\n");
                    if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
                        g_droneData.flight_mode = 2;
                        xSemaphoreGive(xDroneDataMutex);
                    }
                    e2_fan_speed_set(e2_fan_info, 30);
                    break;
                case 'U':  // 上升 (Up) - 增加高度指令（模拟）
                    PRINTF_LOCKED("[Drone] Command: Increase altitude\r\n");
                    // 可模拟控制窗帘机（E3）表示升降动作
                    e3_curtain_position_set(e3_curtain_info, 90);
                    vTaskDelay(pdMS_TO_TICKS(300));
                    e3_curtain_position_set(e3_curtain_info, 50);
                    break;
                case 'D':  // 下降 (Down)
                    PRINTF_LOCKED("[Drone] Command: Decrease altitude\r\n");
                    e3_curtain_position_set(e3_curtain_info, 0); // 假设负值反向
                    vTaskDelay(pdMS_TO_TICKS(300));
                    e3_curtain_position_set(e3_curtain_info, 50);
                    break;
                default:
                    PRINTF_LOCKED("[Drone] Unknown command: %c\r\n", cmd);
                    break;
            }
        }

        /* 获取最新共享数据 */
        if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
            local = g_droneData;
            xSemaphoreGive(xDroneDataMutex);
        }

        /* 温湿度异常报警 */
        uint8_t alarm = 0;
        if (local.temperature > 45.0f || local.temperature < 0.0f ||
            local.humidity > 85.0f || local.humidity < 20.0f) {
            alarm = 1;
        }
        if (alarm != local.alarm_flag) {
            if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
                g_droneData.alarm_flag = alarm;
                xSemaphoreGive(xDroneDataMutex);
            }
            // 报警时风扇全速转动（降温/除湿模拟），LED变红
            if (alarm) {
                e2_fan_speed_set(e2_fan_info, 100);
                // LED红色（通过Display任务也可以，此处直接设置）
                if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                    e1_led_rgb_set(e1_led_info, 100, 0, 0);
                    xSemaphoreGive(xI2CMutex);
                }
            } else {
                // 恢复风扇由档位控制
                int speed = (local.power_gear == 0) ? 0 : (local.power_gear == 1 ? 30 : 60);
                e2_fan_speed_set(e2_fan_info, speed);
            }
        }
        last_temp = local.temperature;
        last_humi = local.humidity;

        /* 飞行时长累计（仅当飞行模式为起飞或悬停） */
        TickType_t now = xTaskGetTickCount();
        if (local.flight_mode == 1 || local.flight_mode == 2) {
            if ((now - lastSecondTick) >= pdMS_TO_TICKS(1000)) {
                lastSecondTick = now;
                if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
                    g_droneData.flight_time++;
                    xSemaphoreGive(xDroneDataMutex);
                }
            }
        } else {
            // 非飞行状态重置计时
            if (g_droneData.flight_time != 0) {
                if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE) {
                    g_droneData.flight_time = 0;
                    xSemaphoreGive(xDroneDataMutex);
                }
            }
            lastSecondTick = now;
        }
    }
}
