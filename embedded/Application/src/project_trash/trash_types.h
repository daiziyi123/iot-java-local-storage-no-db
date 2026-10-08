#ifndef TRASH_TYPES_H
#define TRASH_TYPES_H

#include <stdint.h>

/* 智能垃圾桶项目专用数据结构 */
typedef struct {
    float distance;          // 超声波距离 (cm)
    uint8_t capacity;        // 垃圾容量百分比 (0~100)
    unsigned char  person_detected; // 人体红外检测 (1:有人)
    uint8_t lid_open;        // 桶盖状态 (1:开)
    uint8_t alarm;           // 满溢报警 (1:报警)
    uint8_t compressing;     // 压缩状态 (1:正在压缩)
} TrashData_t;

#endif