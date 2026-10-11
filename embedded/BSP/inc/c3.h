#ifndef C3_H
#define C3_H

#include <stdint.h>
#include <stddef.h>

// 缓冲区大小
#define C3_RECV_BUF_SIZE 256

// WiFi 和 TCP 服务器配置（可根据需要修改）
#define C3_WIFI_SSID     "yuwei70"
#define C3_WIFI_PASS     "yuwei123"
#define C3_TCP_SERVER_IP "192.168.43.10"
#define C3_TCP_SERVER_PORT 8081  // [修复] 原 1025，后端 HTTP 已占 8080，TCP 设备端口改为 8081

// 状态枚举
typedef enum {
    WIFI_STATE_AT_CHECK = 1,
    WIFI_STATE_ATE0,
    WIFI_STATE_CWMODE,
    WIFI_STATE_CWJAP,
    WIFI_STATE_CIFSR,
    WIFI_STATE_CIPSTART,
    WIFI_STATE_RESET
} wifi_state_t;

// 函数声明
uint8_t c3_init(void);
int c3_wifi_net_status_get(char *recv_buf);
int c3_wifi_tcp_init(int wifi_run_state, char *rec_data);
void c3_wifi_tcp_lead(int *wifi_run_state, int *tcp_status);
int c3_wifi_tcp_send(const char *json_data);
uint16_t c3_wifi_tcp_receive(char *rec_data, uint16_t wait_time);
char c3_get_pending_cmd(void);

#endif
