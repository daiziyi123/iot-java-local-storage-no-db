#ifndef CLASSROOM_TYPES_H
#define CLASSROOM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
/* 智能教室系统数据结构 */
typedef struct {
    // 传感器数据
    float temperature;      // 温度 (°C)
    float humidity;         // 湿度 (%)
    float light;            // 照度 (lux)
    unsigned char pir_detected;   // 人体红外 (0/1)
    uint8_t key_pressed;    // 按键触发标志 (0/1) 用于模式切换

    // 系统状态
    uint8_t mode;           // 教学模式: 0-正常, 1-节能, 2-演示
    uint8_t fan_speed;      // 风扇转速 (0~100)
    uint8_t led_brightness; // LED亮度 (0~100)
    uint8_t curtain_pos;    // 窗帘位置 (0~100, 0全关, 100全开)

    // 能耗与使用率统计
    uint32_t occupancy_sec; // 累计有人秒数
    uint32_t total_sec;     // 系统运行总秒数 (用于计算使用率)
    uint32_t fan_runtime;   // 风扇运行秒数
    uint32_t light_runtime; // 灯光运行秒数 (可换算能耗)
    bool manual_lock;
} ClassroomData_t;

#endif
