#include "app_wifi.h"
#include "../app_common.h"
#include "farm_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "c3.h"
#include <string.h>
#include <stdio.h>

extern SemaphoreHandle_t xFarmDataMutex;
extern FarmData_t g_farmData;
extern QueueHandle_t xFarmCommandQueue;

static int tcp_conn = 0;
static uint8_t c3_result;

void vTaskFarmWifi(void *pvParameters)
{
    char recv_buf[200];
    char send_buf[256];
    TickType_t lastReport = xTaskGetTickCount();

    c3_result = c3_init();
    if (!c3_result)
        PRINTF_LOCKED("ESP32-C3 AT Init Failed!\r\n");

    int wifi_state = 1;
    c3_wifi_tcp_lead(&wifi_state, &tcp_conn);

    for (;;) {
        if (!tcp_conn) {
            c3_wifi_tcp_lead(&wifi_state, &tcp_conn);
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        /* 接收远程命令 */
        int len = c3_wifi_tcp_receive(recv_buf, 1000);
        if (len > 0) {
            recv_buf[len] = '\0';
            PRINTF_LOCKED("[Farm] Recv: %s\r\n", recv_buf);
            char *data = strstr(recv_buf, ":");
            if (data) data++;
            else data = recv_buf;

            if (data[0] == 'I') {
                char cmd = 'I';
                xQueueSend(xFarmCommandQueue, &cmd, portMAX_DELAY);
            } else if (data[0] == 'S') {
                char cmd = 'S';
                xQueueSend(xFarmCommandQueue, &cmd, portMAX_DELAY);
            } else if (data[0] == 'R') {
                char cmd = 'R';
                xQueueSend(xFarmCommandQueue, &cmd, portMAX_DELAY);
            } else if (data[0] == 'Q') {
                lastReport = 0; // 立即上报
            }
        }

        /* 定时上报（每5秒） */
        TickType_t now = xTaskGetTickCount();
        if ((now - lastReport) >= pdMS_TO_TICKS(7000)) {
            lastReport = now;
            FarmData_t local;
            if (xSemaphoreTake(xFarmDataMutex, portMAX_DELAY) == pdTRUE) {
                local = g_farmData;
                xSemaphoreGive(xFarmDataMutex);
            }

            snprintf(send_buf, sizeof(send_buf),
                "{\"project\":17,\"name\":\"Smart Farm\",\"light\":%.1f,\"temperature\":%.1f,\"humidity\":%.1f,\"height\":%.1f,\"irrigating\":%d,\"shade_closed\":%d}",
                local.light, local.temperature, local.humidity,
                local.height, local.irrigating, local.shade_closed);
            if(c3_wifi_tcp_send(send_buf))
            PRINTF_LOCKED("[Farm] Report: %s\r\n", send_buf);
            else
            {
                PRINTF_LOCKED("[Drone] Ctrl report FAILED\n");
                tcp_conn = 0; // 触发重连
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}