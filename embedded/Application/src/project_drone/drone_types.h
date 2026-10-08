#ifndef DRONE_TYPES_H
#define DRONE_TYPES_H

#include <stdint.h>

/* 无人机模拟系统数据结构 */
typedef struct {
    float altitude;          // 高度 (cm)，由超声波测得
    float temperature;       // 温度 (°C)
    float humidity;          // 湿度 (%)
    uint8_t power_gear;      // 动力档位：0~2 (低、中、高)
    uint8_t alarm_flag;      // 报警标志：1=温湿度异常
    uint8_t flight_mode;     // 0=待命, 1=起飞, 2=悬停, 3=降落
    uint32_t flight_time;    // 累计飞行秒数
} DroneData_t;

#endif