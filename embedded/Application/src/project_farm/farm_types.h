#ifndef FARM_TYPES_H
#define FARM_TYPES_H

#include <stdint.h>

/* 智能农场项目专用数据结构 */
typedef struct {
    float light;            // 光照强度 (lux)
    float temperature;      // 温度 (°C)
    float humidity;         // 土壤湿度 (%)
    float height;           // 作物高度 (cm，由超声波测得)
    uint8_t irrigating;     // 灌溉状态 (1:灌溉中)
    uint8_t shade_closed;   // 遮阳棚状态 (1:关闭，0:打开)
} FarmData_t;

#endif