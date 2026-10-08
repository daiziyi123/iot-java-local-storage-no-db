#include "app_wifi.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "c3.h" // 沿用ESP32-C3 AT模块
#include "drone_types.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

extern SemaphoreHandle_t xDroneDataMutex;
extern DroneData_t g_droneData;
extern QueueHandle_t xDroneCommandQueue;

static uint8_t wifi_ready = 0;
static int tcp_conn = 0;
static uint8_t c3_result;

void vTaskWifiAT(void *pvParameters)
{
    char recv_buf[200];
    char send_buf[256];
    TickType_t lastReport = xTaskGetTickCount();

    c3_result = c3_init();
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

        /* 接收指令（阻塞1000ms） */
        int len = c3_wifi_tcp_receive(recv_buf, 1000);
        if (len > 0)
        {
            recv_buf[len] = '\0';
            PRINTF_LOCKED("[Drone] Recv: %s\r\n", recv_buf);
            if (strstr(recv_buf, "CLOSED") || strstr(recv_buf, "ERROR")) {
                PRINTF_LOCKED("[Classroom] TCP closed, reconnecting...\n");
                tcp_conn = 0;
                continue;
            }
            /* 解析指令格式：假设 "CMD: X" */
            char *data = strstr(recv_buf, ":");
            if (data)
                data++;
            else
                data = recv_buf;
            char cmd = data[0];
            // 支持的指令：T, L, H, U, D
            if (cmd == 'T' || cmd == 'L' || cmd == 'H' || cmd == 'U' || cmd == 'D')
            {
                xQueueSend(xDroneCommandQueue, &cmd, portMAX_DELAY);
            }
            else
            {
                PRINTF_LOCKED("[Drone] Ignored invalid cmd: %c\r\n", cmd);
            }
        }

        /* 每5秒上报数据 */
        TickType_t now = xTaskGetTickCount();
        if ((now - lastReport) >= pdMS_TO_TICKS(7000))
        {
            lastReport = now;
            DroneData_t local;
            if (xSemaphoreTake(xDroneDataMutex, portMAX_DELAY) == pdTRUE)
            {
                local = g_droneData;
                xSemaphoreGive(xDroneDataMutex);
            }
            snprintf(send_buf, sizeof(send_buf),
                     "{\"project\":25,\"name\":\"Drone\",\"altitude\":%.1f,\"temperature\":%.1f,\"humidity\":%.1f,\"power_gear\":%d,\"alarm_flag\":%d,\"flight_mode\":%d,\"flight_time\":%lu}",
                     local.altitude, local.temperature, local.humidity,
                     local.power_gear, local.alarm_flag, local.flight_mode, local.flight_time);
            if (c3_wifi_tcp_send(send_buf))
            {
                PRINTF_LOCKED("[Drone] Report: %s\r\n", send_buf);
            }
            else
            {
                PRINTF_LOCKED("[Drone] Ctrl report FAILED\n");
                tcp_conn = 0; // 触发重连
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}