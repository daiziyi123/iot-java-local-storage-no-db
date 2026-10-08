#include "app_control.h"
#include "../app_common.h"
#include "../app_hardware.h"
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
static void autofan(const SensorData_t *pData)
{
    if (pData->temperature <= 20)
        e2_fan_speed_set(e2_fan_info, 0);
    else if (pData->temperature > 20 && pData->temperature < 30)
        e2_fan_speed_set(e2_fan_info, 50);
    else
        e2_fan_speed_set(e2_fan_info, 100);
}

static void autocurtain(const SensorData_t *pData)
{
    #define LIGHT_LOW_THRESHOLD      300
    #define LIGHT_MEDIUM_THRESHOLD   500
    if (pData->illuminance <= LIGHT_LOW_THRESHOLD)
        e3_curtain_position_set(e3_curtain_info, 100);
    else if (pData->illuminance < LIGHT_MEDIUM_THRESHOLD)
        e3_curtain_position_set(e3_curtain_info, 50);
    else
        e3_curtain_position_set(e3_curtain_info, 0);
}

void vTaskControl(void *pvParameters)
{
    Command_t cmd;
    SensorData_t localData;
    static bool manualMode = false;   // false=自动模式，true=手动模式

    for (;;) {
        /* 等待命令，超时 200ms 以执行周期自动控制 */
        if (xQueueReceive(xCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE) {
            /* ---------- 处理命令 ---------- */
            switch (cmd.cmd) {
                case 'A':   // LED 红色
                    e1_led_rgb_set(e1_led_info, 100, 0, 0);
                    PRINTF_LOCKED("LED Red ON\r\n");
                    manualMode = true;   // 进入手动模式
                    break;

                case 'B':   // 风扇 50%
                    e2_fan_speed_set(e2_fan_info, 50);
                    PRINTF_LOCKED("Fan speed 50%%\r\n");
                    manualMode = true;
                    break;

                case 'C': { // 查询传感器（不影响模式）
                    if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE) {
                        localData = g_sensorData;
                        xSemaphoreGive(xSensorMutex);
                    }
                    PRINTF_LOCKED("Temp=%.1fC, Humi=%.1f%%, Light=%.0f\r\n",
                           localData.temperature, localData.humidity, localData.illuminance);
                    char buf[16];
                    sprintf(buf, "T:%.1f", localData.temperature);
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e1_tube_str_set(e1_tube_info, buf);
                        xSemaphoreGive(xI2CMutex);
                    }
                    break;
                }

                case 'D':   // 窗帘 100%
                    e3_curtain_position_set(e3_curtain_info, 100);
                    PRINTF_LOCKED("Curtain 100%%\r\n");
                    manualMode = true;
                    break;

                case 'E':   // LED 关闭
                    e1_led_rgb_set(e1_led_info, 0, 0, 0);
                    PRINTF_LOCKED("LED OFF\r\n");
                    manualMode = true;
                    break;

                case 'F':   // 风扇关闭
                    e2_fan_speed_set(e2_fan_info, 0);
                    PRINTF_LOCKED("Fan OFF\r\n");
                    manualMode = true;
                    break;

                case 'G':   // 窗帘 0%
                    e3_curtain_position_set(e3_curtain_info, 0);
                    PRINTF_LOCKED("Curtain 0%%\r\n");
                    manualMode = true;
                    break;

                case 'H':   // 新增：恢复自动模式
                    manualMode = false;
                    PRINTF_LOCKED("Auto mode restored\r\n");
                    break;

                default:
                    PRINTF_LOCKED("Unknown command: %c\r\n", cmd.cmd);
                    break;
            }
        } else {
            /* ---------- 超时 → 执行自动控制（仅在自动模式下） ---------- */
            if (!manualMode) {
                /* 读取最新传感器数据 */
                if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE) {
                    localData = g_sensorData;
                    xSemaphoreGive(xSensorMutex);
                }
                
                autocurtain(&localData);
                autofan(&localData);
                e1_led_rgb_set(e1_led_info, 0, 0, 100);   // 自动模式 LED 蓝色
            }
            // 若处于手动模式，则什么都不做（保持当前状态）
        }
    }
}

