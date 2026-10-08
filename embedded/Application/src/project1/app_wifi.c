#include "app_wifi.h"
#include "../app_common.h"
#include "../app_hardware.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "c3.h"
#include <string.h>
#include <stdio.h>

/* Wi-Fi 模块内部状态 */
static uint8_t c3_result;
static int tcp_conn_established = 0;
static int wifi_run_state = 1;
static char recv_buffer[200];

void vTaskWifiAT(void *pvParameters)
{   
    /* 初始化 ESP32-C3（AT 指令） */
    c3_result = c3_init();
    if (!c3_result)
        PRINTF_LOCKED("ESP32-C3 AT Init Failed!\r\n");

    for (;;) {
        /* 若 TCP 未连接，执行重连流程 */
        if (tcp_conn_established == 0) {
            c3_wifi_tcp_lead(&wifi_run_state, &tcp_conn_established);
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        /* TCP 已连接，阻塞等待接收数据（超时 200ms） */
        int recv_len = c3_wifi_tcp_receive(recv_buffer, 200);
        if (recv_len > 0) {
            recv_buffer[recv_len] = '\0';
            PRINTF_LOCKED("Recv TCP Cmd: %s\r\n", recv_buffer);

            /* 剥离 AT 响应头，取出命令字符 */
            char *data_start = strstr(recv_buffer, ":");
            if (data_start != NULL)
                data_start++;
            else
                data_start = recv_buffer;

            /* 构造命令并发送到队列（纯消息传递） */
            Command_t cmd;
            cmd.cmd   = data_start[0];
            cmd.param = -1;   // 本 demo 未使用参数

            if (xQueueSend(xCommandQueue, &cmd, portMAX_DELAY) != pdPASS) {
               PRINTF_LOCKED("Command queue full!\r\n");
            }
        }

        static TickType_t last_send_time = 0;
        if ((xTaskGetTickCount() - last_send_time) >= pdMS_TO_TICKS(5000)) {
            send_sensor_data_to_server();   // 你实现的发送函数
            last_send_time = xTaskGetTickCount();
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
/* 发送传感器数据到TCP服务器（HTTP POST JSON） */
static void send_sensor_data_to_server(void)
{   SensorData_t localData;
    if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE) {
            localData = g_sensorData;
            xSemaphoreGive(xSensorMutex);
        }
    float temp = localData.temperature;
    float light = localData.illuminance;
    float humi = localData.humidity;
    // 2. 构建 JSON 字符串（注意长度）
    char json_buf[128];
    snprintf(json_buf, sizeof(json_buf),
             "{\"project\": 0,\"name\":\"test\",\"temperature\":%.1f,\"humidity\":%.1f,\"light\":%.1f}",
             temp, humi, light);

    // 3. 构建 HTTP POST 请求（符合服务器解析）
    char request[256];
    c3_wifi_tcp_send(json_buf);
        printf("Upload OK: %s\n", json_buf);

}

