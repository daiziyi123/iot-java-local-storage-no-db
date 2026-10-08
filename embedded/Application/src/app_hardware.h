#ifndef APP_HARDWARE_H
#define APP_HARDWARE_H
#include "i2c.h" 
/* 各外设驱动句柄（在 main.c 中初始化） */
extern i2c_slave_info e1_tube_info;
extern i2c_slave_info e1_led_info;
extern i2c_slave_info s2_illuminance_info;
extern i2c_slave_info e3_curtain_info;
extern i2c_slave_info s8_ths_info;
extern i2c_slave_info e2_fan_info;
extern i2c_slave_info s1_key_info;
extern i2c_slave_info s7_ir_info;
extern i2c_slave_info s6_ultrasonic_info;
#endif
