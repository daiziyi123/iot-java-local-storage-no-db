#ifndef PET_TYPES_H
#define PET_TYPES_H

#include <stdint.h>

/* 宠物屋项目专用数据结构 */
typedef struct {
    float distance;          // 超声波距离 (cm)
    float temperature;       // 温度 (°C)
    float humidity;          // 湿度 (%)
    uint8_t pir_detected;    // 红外触发标志 (0/1)
    uint8_t in_nest;         // 是否在窝 (distance < 40cm)
    uint32_t stay_seconds;   // 累计停留秒数
    uint8_t feed_reminder;   // 投食提醒标志 (1=需要提醒)
} PetData_t;

#endif