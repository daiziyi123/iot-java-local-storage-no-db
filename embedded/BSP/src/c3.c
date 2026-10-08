#include "uart.h"
#include "stdio.h"
#include "string.h"
#include "c3.h"
#include "FreeRTOS.h"
#include "bsp_log.h"
#include "task.h"   // 使用 vTaskDelay
//#define C3_USART_NUM	USART1	// POGOPIN ???? USART
//#define C3_USART_NUM	USART0	// POGOPIN ???? USART

#define C3_USART_NUM	USART5	// POGOPIN ???? USART


// 内部辅助函数（用于发送AT命令并等待响应）
static int at_cmd_send(const char *cmd, const char *expected, uint32_t timeout_ms, char *out_buf, size_t buf_size);
static void c3_check_ipd(const char *buf, int len);

// 初始化
uint8_t c3_init(void)
{
    char recv_buf[C3_RECV_BUF_SIZE] = {0};
    int recv_cnt;

    // 发送 AT 指令测试
    recv_cnt = at_cmd_send("AT", "OK", 500, recv_buf, sizeof(recv_buf));
    if (recv_cnt > 0) {
        PRINTF_LOCKED("[C3] Init OK\n");
        return 1;
    } else {
        PRINTF_LOCKED("[C3] Init Failed\n");
        return 0;
    }
}

// 通用AT命令发送函数
static int at_cmd_send(const char *cmd, const char *expected, uint32_t timeout_ms, char *out_buf, size_t buf_size)
{
    char send_buf[128];
    int ret_len = 0;

    if (!cmd || !expected || !out_buf || buf_size == 0) return -1;

    // 拼装命令（加 \r\n）
    snprintf(send_buf, sizeof(send_buf), "%s\r\n", cmd);
    if (uart_send_bytes(C3_USART_NUM, (uint8_t*)send_buf, strlen(send_buf)) <= 0) {
        return -2;
    }

    // 读取响应
    ret_len = uart_rece_bytes(C3_USART_NUM, (uint8_t*)out_buf, buf_size - 1, timeout_ms);
    if (ret_len > 0) {
        out_buf[ret_len] = '\0';
        // 查找期望字符串
        if (strstr(out_buf, expected) != NULL) {
            return ret_len;
        }
    }
    return 0;
}

// 网络状态检测（返回0：正常，-1：连接断开，-3：模块重启）
int c3_wifi_net_status_get(char *recv_buf)
{
    if (recv_buf == NULL) return 0;

    if (strstr(recv_buf, "ERROR")) {
        // 可能有错误，但进一步判断
    }
    if (strstr(recv_buf, "+IPD,")) {
        // 有数据，表示连接正常
        return 0;
    }
    if (strstr(recv_buf, "CLOSED")) {
        return -1;
    }
    if (strstr(recv_buf, "ready\r\n")) {
        return -3; // 模块重启
    }
    return 0;
}

// TCP 初始化状态机（改进版）
int c3_wifi_tcp_init(int wifi_run_state, char *rec_data)
{
    char recv_buf[C3_RECV_BUF_SIZE] = {0};
    int ret_len;
    int result = -1; // 默认失败

    if (rec_data == NULL) return -1;
    memset(rec_data, 0, C3_RECV_BUF_SIZE);

    switch (wifi_run_state) {
        case WIFI_STATE_AT_CHECK:
            ret_len = at_cmd_send("AT", "OK", 500, recv_buf, sizeof(recv_buf));
            if (ret_len > 0) {
                PRINTF_LOCKED("[C3] AT OK\n");
                result = 0;
            } else {
                PRINTF_LOCKED("[C3] AT FAIL\n");
            }
            break;

        case WIFI_STATE_ATE0:
            ret_len = at_cmd_send("ATE0", "OK", 500, recv_buf, sizeof(recv_buf));
            if (ret_len > 0) {
                PRINTF_LOCKED("[C3] ATE0 OK\n");
                result = 0;
            } else {
                PRINTF_LOCKED("[C3] ATE0 FAIL\n");
            }
            break;

        case WIFI_STATE_CWMODE:
            ret_len = at_cmd_send("AT+CWMODE=1", "OK", 800, recv_buf, sizeof(recv_buf));
            if (ret_len > 0) {
                PRINTF_LOCKED("[C3] CWMODE OK\n");
                result = 0;
            } else {
                PRINTF_LOCKED("[C3] CWMODE FAIL\n");
            }
            break;

        case WIFI_STATE_CWJAP: {
            // 拼接连接WiFi的命令
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", C3_WIFI_SSID, C3_WIFI_PASS);
            // 等待较长，因为连接可能需要几秒，我们循环等待
            int try_cnt = 0;
            while (try_cnt < 20) { // 最多10秒
                ret_len = at_cmd_send(cmd, "OK", 500, recv_buf, sizeof(recv_buf));
                if (ret_len > 0) {
                    if (strstr(recv_buf, "OK")) {
                        PRINTF_LOCKED("[C3] WiFi Connected\n");
                        result = 0;
                        break;
                    }
                }
                // 检查是否有ERROR或失败
                if (strstr(recv_buf, "ERROR")) {
                   PRINTF_LOCKED("[C3] WiFi Connect ERROR\n");
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(500));
                try_cnt++;
            }
            if (result != 0) {
                PRINTF_LOCKED("[C3] WiFi Connect FAIL\n");
            }
            break;
        }

        case WIFI_STATE_CIFSR:
            ret_len = at_cmd_send("AT+CIFSR", "OK", 800, recv_buf, sizeof(recv_buf));
            if (ret_len > 0) {
                // 检查是否获取到有效IP（非0.0.0.0）
                if (strstr(recv_buf, "0.0.0.0") == NULL) {
                    PRINTF_LOCKED("[C3] CIFSR OK (IP got)\n");
                    result = 0;
                } else {
                    PRINTF_LOCKED("[C3] CIFSR Invalid IP\n");
                }
            } else {
                PRINTF_LOCKED("[C3] CIFSR FAIL\n");
            }
            break;

        case WIFI_STATE_CIPSTART: {
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d", C3_TCP_SERVER_IP, C3_TCP_SERVER_PORT);
            ret_len = at_cmd_send(cmd, "CONNECT", 2000, recv_buf, sizeof(recv_buf));
            if (ret_len > 0) {
                // 可能收到 "CONNECT" 或 "OK"
                if (strstr(recv_buf, "CONNECT") || strstr(recv_buf, "OK")) {
                    PRINTF_LOCKED("[C3] TCP Connected\n");
                    result = 0;
                } else {
                    PRINTF_LOCKED("[C3] TCP Connect FAIL\n");
                }
            } else {
                PRINTF_LOCKED("[C3] TCP Connect timeout\n");
            }
            break;
        }

        case WIFI_STATE_RESET:
            uart_send_bytes(C3_USART_NUM, (uint8_t*)"AT+RST\r\n", 8);
            vTaskDelay(pdMS_TO_TICKS(3000));
            PRINTF_LOCKED("[C3] Module Reset\n");
            result = -2; // 特殊返回表示复位执行
            break;

        default:
            break;
    }

    // 复制响应数据（如有）
    memcpy(rec_data, recv_buf, sizeof(recv_buf));
    return result;
}

// 主循环调用，处理WiFi连接状态机
void c3_wifi_tcp_lead(int *wifi_run_state, int *tcp_status)
{
    static int retry_cnt = 0;
    static int was_connected = 0;  // 记录是否曾经连上过
    char rec_buf[C3_RECV_BUF_SIZE] = {0};
    int ret;

    // 只在 tcp_status == 0 表示未连接时执行初始化
    if (*tcp_status == 0) {
        // 之前连上过但现在断了 → 先复位 C3 模块清理状态
        if (was_connected)
        {
            PRINTF_LOCKED("[C3] Reconnecting, reset module...\n");
            uart_send_bytes(C3_USART_NUM, (uint8_t*)"AT+RST\r\n", 8);
            vTaskDelay(pdMS_TO_TICKS(2000));
            *wifi_run_state = WIFI_STATE_AT_CHECK;
            was_connected = 0;
        }
        ret = c3_wifi_tcp_init(*wifi_run_state, rec_buf);
        if (ret == 0) {
            // 成功，推进状态
            if (*wifi_run_state == WIFI_STATE_CIPSTART) {
                // 所有步骤完成，TCP连接成功
                *tcp_status = 1;
                *wifi_run_state = WIFI_STATE_AT_CHECK; // 重置到初始状态（或保持）
                retry_cnt = 0;
                was_connected = 1;
                PRINTF_LOCKED("[C3] All init done\n");
            } else {
                // 移到下一个状态
                (*wifi_run_state)++;
                retry_cnt = 0;
                PRINTF_LOCKED("[C3]connect next init\n");
            }
        } else if (ret == -2) {
            // 复位操作，状态不变
            // 可能等待模块重启后重新从 AT 开始
            *wifi_run_state = WIFI_STATE_AT_CHECK;
            retry_cnt = 0;
            PRINTF_LOCKED("[C3] connect tcp again\n");
        } else {
            // 失败，检查网络状态
            int net_st = c3_wifi_net_status_get(rec_buf);
            if (net_st == -1) {
                // 连接已关闭，需要重新连接（可能从头开始或复位）
               PRINTF_LOCKED("[C3] Connection closed, reconnecting...\n");
                *wifi_run_state = WIFI_STATE_RESET;
                retry_cnt++;
            } else if (net_st == -3) {
                // 模块重启，从头开始
                PRINTF_LOCKED("[C3] from head again...\n");
                *wifi_run_state = WIFI_STATE_AT_CHECK;
                retry_cnt++;
            } else {
                // 其他错误，重试当前步骤
                PRINTF_LOCKED("[C3] else error...\n");
                retry_cnt++;
                if (retry_cnt >= 3) {
                    // 多次失败，复位
                    PRINTF_LOCKED("[C3] get back...\n");
                    *wifi_run_state = WIFI_STATE_RESET;
                    retry_cnt = 0;
                }
            }
            // 如果 tcp_status 仍为 -1，保持等待下次重试
        }
    }
}

// TCP 发送数据
int c3_wifi_tcp_send(const char *json_data)
{ 
    char recv_buf[200] = {0};
   char send_buf[512]= {0};
    char cmd_buf[128];
    int json_len = strlen(json_data);
    int recv_cnt;

    // 构建 HTTP POST 请求
    snprintf(send_buf, sizeof(send_buf),
        "POST /device/update HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        C3_TCP_SERVER_IP, C3_TCP_SERVER_PORT, json_len, json_data);

    int total_len = strlen(send_buf);
    //PRINTF_LOCKED("%d\n",total_len);
    // 发送 AT+CIPSEND 命令
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+CIPSEND=%d\r\n", total_len);
    uart_send_bytes(C3_USART_NUM, (uint8_t*)cmd_buf, strlen(cmd_buf));

    // 等待 '>' 提示符，超时 500ms
    recv_cnt = uart_rece_bytes(C3_USART_NUM, (uint8_t*)recv_buf, sizeof(recv_buf) - 1, 500);
    if (recv_cnt > 0) c3_check_ipd(recv_buf, recv_cnt);
    if (recv_cnt <= 0 || strstr(recv_buf, ">") == NULL) {
        PRINTF_LOCKED("[C3] CIPSEND prompt not received\n");
        return 0;
    }

    // 发送 HTTP 数据
    uart_send_bytes(C3_USART_NUM, (uint8_t*)send_buf, total_len);

    // 等待发送完成确认
    memset(recv_buf, 0, sizeof(recv_buf));
    recv_cnt = uart_rece_bytes(C3_USART_NUM, (uint8_t*)recv_buf, sizeof(recv_buf) - 1, 1000);
    if (recv_cnt > 0)
    {
        c3_check_ipd(recv_buf, recv_cnt);
        if (strstr(recv_buf, "OK") != NULL) {
            return 1;
        }
    }
    PRINTF_LOCKED("[C3] Send failed: %s\n", recv_buf);
    return 0;
}

// 保存从 +IPD 消息中提取的命令
static char g_pending_cmd = 0;

// 扫描缓冲区中是否包含 +IPD 数据，提取命令字符
static void c3_check_ipd(const char *buf, int len)
{
    if (len <= 0 || buf == NULL) return;
    const char *ipd = strstr(buf, "+IPD,");
    if (ipd)
    {
        const char *colon = strstr(ipd, ":");
        if (colon && colon[1] != '\0')
        {
            g_pending_cmd = colon[1];  // 保存 : 后面的第一个字符
        }
    }
}

// 获取待处理命令
char c3_get_pending_cmd(void)
{
    char cmd = g_pending_cmd;
    g_pending_cmd = 0;
    return cmd;
}

// TCP 接收数据（返回接收字节数）
uint16_t c3_wifi_tcp_receive(char *rec_data, uint16_t wait_time)
{
    if (rec_data == NULL) return 0;
    uint16_t len = uart_rece_bytes(C3_USART_NUM, (uint8_t*)rec_data, C3_RECV_BUF_SIZE - 1, wait_time);
    if (len > 0)
    {
        rec_data[len] = '\0';
        c3_check_ipd(rec_data, len);
    }
    return len;
}


