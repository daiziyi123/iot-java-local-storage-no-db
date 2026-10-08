#include "app_wifi.h"
#include "../app_common.h"
#include "FreeRTOS.h"
#include "c3.h"
#include "pet_types.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

extern SemaphoreHandle_t xPetDataMutex;
extern PetData_t g_petData;
extern QueueHandle_t xPetCommandQueue;

static int tcp_conn = 0;
static uint8_t c3_result;
void vTaskWifiAT(void *pvParameters)
{
    char recv_buf[200];
    char send_buf[256];
    TickType_t lastReport = xTaskGetTickCount();

    c3_result = c3_init();
    if (!c3_result)
        PRINTF_LOCKED("ESP32-C3 AT Init Failed!\r\n");
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

        int len = c3_wifi_tcp_receive(recv_buf, 1000);
        if (len > 0)
        {
            recv_buf[len] = '\0';
            PRINTF_LOCKED("[Pet] Recv: %s\r\n", recv_buf);
            if (strstr(recv_buf, "CLOSED") || strstr(recv_buf, "ERROR")) {
                PRINTF_LOCKED("[Classroom] TCP closed, reconnecting...\n");
                tcp_conn = 0;
                continue;
            }
            char *data = strstr(recv_buf, ":");
            if (data)
                data++;
            else
                data = recv_buf;
            if (data[0] == 'H')
            {
                char cmd = 'H';
                xQueueSend(xPetCommandQueue, &cmd, portMAX_DELAY);
            }
            else if (data[0] == 'Q')
            {
                char cmd = 'Q';
                xQueueSend(xPetCommandQueue, &cmd, portMAX_DELAY);
            }
        }

        TickType_t now = xTaskGetTickCount();
        if ((now - lastReport) >= pdMS_TO_TICKS(7000))
        {
            lastReport = now;
            PetData_t local;
            if (xSemaphoreTake(xPetDataMutex, portMAX_DELAY) == pdTRUE)
            {
                local = g_petData;
                xSemaphoreGive(xPetDataMutex);
            }
            snprintf(send_buf, sizeof(send_buf),
                     "{\"project\": 13,\"name\":\"PetHouse\",\"distance\":%.1f,\"temperature\":%.1f,\"humidity\":%.1f,\"in_nest\":%d,\"stay_seconds\":%lu,\"feed_reminder\":%d}",
                     local.distance, local.temperature, local.humidity, local.in_nest, local.stay_seconds, local.feed_reminder);

            if (c3_wifi_tcp_send(send_buf))
                PRINTF_LOCKED("[Pet] Report: %s\r\n", send_buf);
            else
            {
                PRINTF_LOCKED("[Drone] Ctrl report FAILED\n");
                tcp_conn = 0; // 触发重连
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}