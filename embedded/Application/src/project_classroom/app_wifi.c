#include "app_wifi.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "c3.h" // 假设的WiFi模块驱动
#include "classroom_types.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

extern SemaphoreHandle_t xClassroomMutex;
extern ClassroomData_t g_classroomData;
extern QueueHandle_t xClassroomCommandQueue;

static int tcp_conn = 0;
static uint8_t send_fail_cnt = 0; // 连续发送失败计数
void vTaskWifiAT(void *pvParameters)
{
    char recv_buf[200];
    TickType_t lastReportEnv = xTaskGetTickCount();                        // 环境数据上报时间戳
    TickType_t lastReportCtrl = xTaskGetTickCount() - pdMS_TO_TICKS(3500); // 控制数据偏移3.5秒

    uint8_t c3_result = c3_init();
    if (!c3_result)
    {
        PRINTF_LOCKED("ESP32-C3 AT Init Failed!\r\n");
    }
    int wifi_state = 1;
    c3_wifi_tcp_lead(&wifi_state, &tcp_conn);

    for (;;)
    {
        if (!tcp_conn)
        {
            c3_wifi_tcp_lead(&wifi_state, &tcp_conn);
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // 检查从 c3_wifi_tcp_send 中提取到的 +IPD 命令
        char pending = c3_get_pending_cmd();
        if (pending && (pending == 'M' || pending == 'F' || pending == 'L' || pending == 'O'))
        {
            xQueueSend(xClassroomCommandQueue, &pending, pdMS_TO_TICKS(100));
        }

        // 接收远程命令
        int len = c3_wifi_tcp_receive(recv_buf, 1000);
        if (len > 0)
        {
            recv_buf[len] = '\0';
            PRINTF_LOCKED("[Classroom] Recv: %s\r\n", recv_buf);
            if (strstr(recv_buf, "CLOSED") || strstr(recv_buf, "ERROR"))
            {
                PRINTF_LOCKED("[Classroom] TCP closed, reconnecting...\n");
                tcp_conn = 0;
                continue;
            }
            // 解析命令，支持 'M' 切换模式, 'F' 风扇, 'L' 灯光,' O' 自动
            char *data = strstr(recv_buf, ":");
            if (data)
                data++;
            else
                data = recv_buf;
            char cmd = data[0];
            if (cmd == 'M' || cmd == 'F' || cmd == 'L' || cmd == 'O')
            {
                if (xQueueSend(xClassroomCommandQueue, &cmd, pdMS_TO_TICKS(100)) != pdPASS)
                {
                    PRINTF_LOCKED("[Classroom] Command queue full, drop cmd: %c\r\n", cmd);
                }
            }
            else {
                PRINTF_LOCKED("[WiFi] Unknown remote cmd: %c\r\n", cmd);
            }
        }

        // -------- 定时上报1：控制与统计数据（每7秒） --------
        TickType_t now = xTaskGetTickCount();
        if ((now - lastReportEnv) >= pdMS_TO_TICKS(7000))
        {
            lastReportEnv = now;
            ClassroomData_t local;
            if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
            {
                local = g_classroomData;
                xSemaphoreGive(xClassroomMutex);
            }

            char send_buf[256];
            snprintf(send_buf, sizeof(send_buf),
                     "{\"project\":20,\"name\":\"Classroom\","
                     "\"temperature\":%.1f,\"humidity\":%.1f,\"light\":%.0f,\"pir_detected\":%d}",
                     local.temperature, local.humidity, local.light, local.pir_detected);

            if (c3_wifi_tcp_send(send_buf))
            {
                PRINTF_LOCKED("[Classroom] Env report sent OK\n");
                send_fail_cnt = 0;
            }
            else
            {
                PRINTF_LOCKED("[Classroom] Env report FAILED, reconnecting...\n");
                tcp_conn = 0;
                send_fail_cnt = 0;
                continue;
            }
        }

        // -------- 定时上报2：控制与统计数据（每7秒，相位偏移秒） --------
        if ((now - lastReportCtrl) >= pdMS_TO_TICKS(7000))
        {
            lastReportCtrl = now;
            ClassroomData_t local;
            uint32_t usage = 0;

            if (xSemaphoreTake(xClassroomMutex, portMAX_DELAY) == pdTRUE)
            {
                local = g_classroomData;
                xSemaphoreGive(xClassroomMutex);
            }

            if (local.total_sec > 0)
            {
                usage = (local.occupancy_sec * 100) / local.total_sec;
            }

            char send2_buf[256];
            snprintf(send2_buf, sizeof(send2_buf),
                     "{\"project\":20,\"name\":\"Classroom\","
                     "\"mode\":%d,\"fan_speed\":%d,\"led_brightness\":%d,"
                     "\"curtain_pos\":%d,\"usage\":%lu,\"fan_runtime\":%lu,\"light_runtime\":%lu}",
                     local.mode, local.fan_speed, local.led_brightness,
                     local.curtain_pos, usage, local.fan_runtime, local.light_runtime);

            if (c3_wifi_tcp_send(send2_buf))
            {
                PRINTF_LOCKED("[Classroom] Ctrl report sent OK\n");
                send_fail_cnt = 0;
            }
            else
            {
                PRINTF_LOCKED("[Classroom] Ctrl report FAILED, reconnecting...\n");
                tcp_conn = 0;
                send_fail_cnt = 0;
                continue;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
