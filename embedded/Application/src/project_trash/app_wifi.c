#include "app_wifi.h"
#include "../app_common.h"
#include "trash_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "c3.h"
#include <string.h>
#include <stdio.h>

extern SemaphoreHandle_t xTrashDataMutex;
extern TrashData_t g_trashData;
extern QueueHandle_t xTrashCommandQueue;

static int tcp_conn = 0;
static uint8_t c3_result;

void vTaskTrashWifi(void *pvParameters)
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
            PRINTF_LOCKED("[Trash] Recv: %s\r\n", recv_buf);
            char *data = strstr(recv_buf, ":");
            if (data) data++;
            else data = recv_buf;

            if (data[0] == 'O') {
                char cmd = 'O';
                xQueueSend(xTrashCommandQueue, &cmd, portMAX_DELAY);
            } else if (data[0] == 'C') {
                char cmd = 'C';
                xQueueSend(xTrashCommandQueue, &cmd, portMAX_DELAY);
            } else if (data[0] == 'R') {
                char cmd = 'R';
                xQueueSend(xTrashCommandQueue, &cmd, portMAX_DELAY);
            }
        }

        /* 定时上报（每5秒） */
        TickType_t now = xTaskGetTickCount();
        if ((now - lastReport) >= pdMS_TO_TICKS(7000)) {
            lastReport = now;
            TrashData_t local;
            if (xSemaphoreTake(xTrashDataMutex, portMAX_DELAY) == pdTRUE) {
                local = g_trashData;
                xSemaphoreGive(xTrashDataMutex);
            }

            snprintf(send_buf, sizeof(send_buf),
                "{\"project\":16,\"name\":\"SmartTrash\",\"capacity\":%d,\"distance\":%.1f,\"person_detected\":%d,\"lid_open\":%d,\"alarm\":%d,\"compressing\":%d}",
                local.capacity, local.distance, local.person_detected,
                local.lid_open, local.alarm, local.compressing);

            c3_wifi_tcp_send(send_buf);
            PRINTF_LOCKED("[Trash] Report: %s\r\n", send_buf);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}