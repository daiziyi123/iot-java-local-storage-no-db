#include "app_control.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "classroom_types.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>

/* 执行器驱动（假设） */
#include "e1.h" // LED+数码管
#include "e2.h" // 风扇
#include "e3.h" // 窗帘

extern SemaphoreHandle_t xClassroomMutex;
extern ClassroomData_t g_classroomData;
extern QueueHandle_t xClassroomCommandQueue;

void vTaskControl(void *pvParameters)
{
    char cmd;
    ClassroomData_t local;
    TickType_t lastSecondTick = xTaskGetTickCount();

    for (;;)
    {
        /* 处理命令（超时200ms） */
        if (xQueueReceive(xClassroomCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE)
        {
            switch (cmd)
            {
            case 'M': // 切换模式
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.mode = (g_classroomData.mode + 1) % 3;
                    xSemaphoreGive(xClassroomMutex);
                    PRINTF_LOCKED("[Classroom] Mode changed to %d\r\n", g_classroomData.mode);
                }
                break;
            case 'F': // 远程手动控制风扇：开关切换，进入手动锁定
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.fan_speed = (g_classroomData.fan_speed == 0) ? 80 : 0;
                    g_classroomData.manual_lock = true; // 标记进入手动模式，自动逻辑不再覆盖
                    xSemaphoreGive(xClassroomMutex);
                    e2_fan_speed_set(e2_fan_info, g_classroomData.fan_speed);
                    PRINTF_LOCKED("[Classroom] Remote manual fan speed: %d, locked auto control\r\n", g_classroomData.fan_speed);
                }
                break;
            case 'L': // 远程手动控制灯光：开关切换，进入手动锁定
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.led_brightness = (g_classroomData.led_brightness == 0) ? 80 : 0;
                    g_classroomData.manual_lock = true; 
                    xSemaphoreGive(xClassroomMutex);
                    int8_t bright = g_classroomData.led_brightness * 255 / 100;
                    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
                        e1_led_rgb_set(e1_led_info, bright, bright, bright);
                        xSemaphoreGive(xI2CMutex);
                    }
                    PRINTF_LOCKED("[Classroom] Remote manual led brightness: %d, locked auto control\r\n", g_classroomData.led_brightness);
                }
                break;
            case 'O': // 解除手动锁定，恢复自动模式
                g_classroomData.manual_lock = false;
                PRINTF_LOCKED("[Classroom] Auto control restored, manual lock released\r\n");
                break;
            default:
                PRINTF_LOCKED("[Classroom] Unknown cmd: %c\r\n", cmd);
                break;
            }
        }

        /* 读取最新共享数据 */
        if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
        {
            local = g_classroomData;
            xSemaphoreGive(xClassroomMutex);
        }

        /* 处理按键切换模式（若检测到按键按下） */
        if (local.key_pressed)
        {
            if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
            {
                g_classroomData.mode = (g_classroomData.mode + 1) % 3;
                g_classroomData.key_pressed = 0; // 清除标志
                xSemaphoreGive(xClassroomMutex);
                PRINTF_LOCKED("[Classroom] Key mode to %d\r\n", g_classroomData.mode);
                local = g_classroomData;
            }
        
        }

        /* ---- 自动控制逻辑 ---- */

        // 1. 无人检测：若非手动，且无人，关闭所有设备（强制）
    if (!g_classroomData.manual_lock)
    {
        if (!local.pir_detected)
        {
            // 关闭风扇
            if (local.fan_speed != 0)
            {
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.fan_speed = 0;
                    local.fan_speed = 0;
                    xSemaphoreGive(xClassroomMutex);
                }
                e2_fan_speed_set(e2_fan_info, 0);
            }
            // 关闭灯光
            if (local.led_brightness != 0)
            {
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.led_brightness = 0;
                    local.led_brightness = 0;
                    xSemaphoreGive(xClassroomMutex);
                }
                e1_led_rgb_set(e1_led_info, 0, 0, 0); // 全灭
            }
            // 关闭窗帘（合上）
            if (local.curtain_pos != 0)
            {
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.curtain_pos = 0;
                    local.curtain_pos = 0;
                    xSemaphoreGive(xClassroomMutex);
                }
                e3_curtain_position_set(e3_curtain_info, 0);
            }
            // 无人时，不累计占用时间，但总时间仍累计
        }
        else
        {
            // 有人：根据模式执行自动调控

            // 光照自动调控：根据照度调节LED亮度或窗帘
            // 这里简单：若光照 < 300 lux 则开灯，否则关灯；同时窗帘根据光照调节（亮则关窗帘，暗则开窗帘）
            // 但不同模式可不同策略
            uint8_t target_led = 0;
            uint8_t target_curtain = 50; // 默认半开
            switch (local.mode)
            {
            case 0: // 正常模式
                if (local.light < 300)
                {
                    target_led = 80;
                    target_curtain = 20; // 窗帘关小
                }
                else if (local.light < 600)
                {
                    target_led = 40;
                    target_curtain = 50;
                }
                else
                {
                    target_led = 0;
                    target_curtain = 80; // 窗帘打开
                }
                break;
            case 1: // 节能模式：更少灯光
                if (local.light < 200)
                {
                    target_led = 50;
                    target_curtain = 30;
                }
                else
                {
                    target_led = 0;
                    target_curtain = 70;
                }
                break;
            case 2: // 演示模式：保持明亮
                target_led = 100;
                target_curtain = 30;
                break;
            default:
                break;
            }

            // 应用LED亮度（若变化）
            if (local.led_brightness != target_led)
            {
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.led_brightness = target_led;
                    local.led_brightness = target_led;
                    xSemaphoreGive(xClassroomMutex);
                }
                // 设置LED颜色（白色，亮度为target_led）
                if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE)
                {   uint8_t bright = target_led * 255 / 100;
                    e1_led_rgb_set(e1_led_info, bright, bright, bright);
                    xSemaphoreGive(xI2CMutex);
                }
            }

            // 应用窗帘位置（若变化）
            if (local.curtain_pos != target_curtain)
            {
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.curtain_pos = target_curtain;
                    local.curtain_pos = target_curtain;
                    xSemaphoreGive(xClassroomMutex);
                }
                e3_curtain_position_set(e3_curtain_info, target_curtain);
            }

            // 温度自动调控：根据温度调节风扇转速
            uint8_t target_fan = 0;
            if (local.temperature > 28.0f)
            {
                target_fan = 100;
            }
            else if (local.temperature > 24.0f)
            {
                target_fan = 50;
            }
            else
            {
                target_fan = 0;
            }
            // 节能模式下可降低风速
            if (local.mode == 1)
            {
                target_fan = target_fan * 0.6;
            }
            if (local.fan_speed != target_fan)
            {
                if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_classroomData.fan_speed = target_fan;
                    local.fan_speed = target_fan;
                    xSemaphoreGive(xClassroomMutex);
                }
                e2_fan_speed_set(e2_fan_info, target_fan);
            }
        }
    }
        /* ---- 能耗统计（每秒累计） ---- */
        TickType_t now = xTaskGetTickCount();
        if ((now - lastSecondTick) >= pdMS_TO_TICKS(1000))
        {
            lastSecondTick = now;
            if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
            {
                g_classroomData.total_sec++;
                if (local.pir_detected)
                {
                    g_classroomData.occupancy_sec++;
                }
                if (local.fan_speed > 0)
                {
                    g_classroomData.fan_runtime++;
                }
                if (local.led_brightness > 0)
                {
                    g_classroomData.light_runtime++;
                }
                xSemaphoreGive(xClassroomMutex);
            }
        }
    }
}
