/**
 * @file app_wifi.c
 * @brief WiFi 通信任务（智能窗户系统）– 参考教室版重构
 */

#include "app_wifi.h"
#include "../app_common.h"
#include "window_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "c3.h"
#include <string.h>
#include <stdio.h>

/* 外部资源声明 */
extern SemaphoreHandle_t xWindowMutex;
extern windowData_t g_windowData;
extern QueueHandle_t xWindowCommandQueue;

/* 静态变量 */
static int tcp_conn = 0;          // TCP 连接状态（1=已连接）
static uint8_t send_fail_cnt = 0; // 连续发送失败计数

/**
 * @brief WiFi 任务入口
 */
void vTaskWifiAT(void *pvParameters)
{
    char recv_buf[200];
    TickType_t lastReportEnv = xTaskGetTickCount();                // 环境数据上报时间戳
    TickType_t lastReportCtrl = xTaskGetTickCount() - pdMS_TO_TICKS(3500); // 控制数据偏移3.5秒

    /* 初始化 ESP32-C3 AT 模块 */
    uint8_t c3_result = c3_init();
    if (!c3_result) {
        PRINTF_LOCKED("[WiFi] ESP32-C3 AT Init Failed!\r\n");
    }

    int wifi_state = 1;
    c3_wifi_tcp_lead(&wifi_state, &tcp_conn);

    for (;;) {
        /* -------------------- 1. 处理 TCP 连接状态 -------------------- */
        if (!tcp_conn) {
            c3_wifi_tcp_lead(&wifi_state, &tcp_conn);
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        /* -------------------- 2. 接收远程命令 -------------------- */
        int len = c3_wifi_tcp_receive(recv_buf, 1000);
        if (len > 0) {
            recv_buf[len] = '\0';
            PRINTF_LOCKED("[WiFi] Recv: %s\r\n", recv_buf);

            /* 检测 TCP 断开 */
            if (strstr(recv_buf, "CLOSED") || strstr(recv_buf, "ERROR")) {
                PRINTF_LOCKED("[WiFi] TCP closed, reconnecting...\n");
                tcp_conn = 0;
                continue;
            }

            /* 解析命令：支持 "CMD:..." 或直接首字符 */
            char *data = strstr(recv_buf, ":");
            if (data) {
                data++;  // 跳过 ':'
            } else {
                data = recv_buf;
            }

            char cmd = data[0];
            if (cmd == 'C' || cmd == 'O' || cmd == 'A') {
                /* 发送到控制任务队列（带超时，避免阻塞） */
                if (xQueueSend(xWindowCommandQueue, &cmd, pdMS_TO_TICKS(100)) != pdPASS) {
                    PRINTF_LOCKED("[WiFi] Command queue full, drop cmd: %c\r\n", cmd);
                }
            } else if (cmd == 'Q') {
                /* 立即上报数据（将两个时间戳同时置零） */
                lastReportEnv = 0;
                lastReportCtrl = 0;
                PRINTF_LOCKED("[WiFi] Immediate report triggered\r\n");
            } else {
                PRINTF_LOCKED("[WiFi] Unknown remote cmd: %c\r\n", cmd);
            }
        }

        /* -------------------- 3. 定时上报1：环境数据（每7秒） -------------------- */
        TickType_t now = xTaskGetTickCount();
        if ((now - lastReportEnv) >= pdMS_TO_TICKS(7000)) {
            lastReportEnv = now;
            windowData_t local;

            if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                local = g_windowData;
                xSemaphoreGive(xWindowMutex);
            }

            char send_buf[256];
            snprintf(send_buf, sizeof(send_buf),
                     "{\"project\":15,\"name\":\"Window\","
                     "\"temperature\":%.1f,\"humidity\":%.1f,\"light\":%.0f,\"pir_detected\":%d}",
                     local.temperature, local.humidity, local.light, local.pir_detected);

            if (c3_wifi_tcp_send(send_buf)) {
                PRINTF_LOCKED("[WiFi] Env report sent OK\n");
                send_fail_cnt = 0;   // 发送成功，清零
            } else {
                PRINTF_LOCKED("[WiFi] Env report FAILED\n");
                send_fail_cnt++;
                if (send_fail_cnt >= 3) {
                    PRINTF_LOCKED("[WiFi] Too many send failures, reconnect\n");
                    tcp_conn = 0;
                    send_fail_cnt = 0;
                }
            }
        }

        /* -------------------- 4. 定时上报2：控制与统计（每7秒，偏移3.5秒） -------------------- */
        if ((now - lastReportCtrl) >= pdMS_TO_TICKS(7000)) {
            lastReportCtrl = now;
            windowData_t local;
            uint32_t usage = 0;

            if (xSemaphoreTake(xWindowMutex, portMAX_DELAY) == pdTRUE) {
                local = g_windowData;
                xSemaphoreGive(xWindowMutex);
            }

            if (local.total_sec > 0) {
                usage = (local.occupancy_sec * 100) / local.total_sec;
            }

            char send2_buf[256];
            snprintf(send2_buf, sizeof(send2_buf),
                     "{\"project\":15,\"name\":\"Window\","
                     "\"led_brightness\":%d,\"curtain_pos\":%d,"
                     "\"usage_rate\":%lu,\"curtain_runtime\":%d}",
                     local.led_brightness, local.curtain_pos,
                     usage, local.curtain_runtime);

            if (c3_wifi_tcp_send(send2_buf)) {
                PRINTF_LOCKED("[WiFi] Ctrl report sent OK\n");
                send_fail_cnt = 0;
            } else {
                PRINTF_LOCKED("[WiFi] Ctrl report FAILED\n");
                send_fail_cnt++;
                if (send_fail_cnt >= 3) {
                    PRINTF_LOCKED("[WiFi] Too many send failures, reconnect\n");
                    tcp_conn = 0;
                    send_fail_cnt = 0;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}