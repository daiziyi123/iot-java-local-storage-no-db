#ifndef WINDOW_TYPES_H
#define WINDOW_TYPES_H

#include <stdint.h>

/* 智能教室系统数据结构 */
typedef struct {
    // 传感器数据
    float temperature;      // 温度 (°C)
    float humidity;         // 湿度 (%)
    float light;            // 照度 (lux)
    unsigned char pir_detected;   // 人体红外 (0/1)

    // 系统状态
    uint8_t led_brightness; // LED亮度 (0~100)
    uint8_t curtain_pos;    // 窗户位置 (0~100, 0全关, 100全开)
    uint8_t opening;        //窗户是否打开
    uint8_t ifauto;         //是否为自动模式
    // 能耗与使用率统计
    uint32_t occupancy_sec; // 累计有人秒数
    uint32_t total_sec;     // 系统运行总秒数 (用于计算使用率)
    uint32_t curtain_runtime; // 窗户运行秒数 (可换算能耗)
} windowData_t;

#endif