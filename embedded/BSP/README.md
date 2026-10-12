# BSP 模块与开发板对应关系

本文档说明 `BSP/` 目录下每个驱动文件与开发板硬件模块的对应关系。

---

## 命名规则

| 前缀 | 含义 | 示例 |
|------|------|------|
| `e` | 执行器（Executor） | e1=数码管+LED，e2=风扇，e3=窗帘 |
| `s` | 传感器（Sensor） | s1=按键，s8=温湿度 |
| `c` | 通信模块（Communication） | c3=WiFi(ESP32-C3) |
| `u` | 子板基础单元（Unit） | u1=板载LED+定时器 |
| 其他 | 基础服务 | i2c、uart、delay |

---

## 执行器（E 系列）

| 文件 | 开发板模块 | 驱动芯片 | 接口 | 主要函数 |
|------|-----------|---------|------|---------|
| `e1.c` / `e1.h` | 数码管 + RGB LED | PCA9685(PWM) + HT16K33(数码管) | I2C | `e1_tube_str_set()` 显示字符串<br>`e1_led_rgb_set(r,g,b)` 设置RGB颜色 |
| `e2.c` / `e2.h` | 风扇 | PCA9685 PWM | I2C | `e2_fan_speed_set(speed)` 设置转速 0~100% |
| `e3.c` / `e3.h` | 窗帘电机 | — | I2C | `e3_curtain_position_set(pos)` 设置位置 0~100 |

---

## 传感器（S 系列）

| 文件 | 开发板模块 | 接口 | 主要函数 |
|------|-----------|------|---------|
| `s1.c` / `s1.h` | 按键矩阵 | GPIO | `s1_key_value_get()` 读取键值 |
| `s2.c` / `s2.h` | 光照传感器 + 温湿度 + IMU(加速度/角速度) | I2C | `s2_illuminance_value_get()` 照度(lux)<br>`s2_ths_value_get()` 温湿度<br>`s2_imu_value_get()` 加速度/角速度 |
| `s5.c` / `s5.h` | NFC (MS523，兼容MFRC522) | I2C | `s5_nfc_request()` 寻卡<br>`s5_nfc_anticoll()` 防冲撞<br>`s5_nfc_select()` 选卡<br>`s5_nfc_auth()` 认证<br>`s5_nfc_read()`/`s5_nfc_write()` 读写卡<br>`s5_nfc_halt()` 休眠 |
| `s6.c` / `s6.h` | 超声波测距 | GPIO | `s6_ultrasonic_distance_get()` 距离(mm) |
| `s7.c` / `s7.h` | 人体红外感应 | GPIO | `s7_ir_status_get()` 0=无人 1=有人 |
| `s8.c` / `s8.h` | 温湿度传感器 | I2C | `s8_ths_value_get()` 温度(°C)/湿度(%) |
| `s11.c` / `s11.h` | 称重传感器 | I2C | `s11_scale_weight_get()` 重量 |

---

## 通信模块（C 系列）

| 文件 | 开发板模块 | 接口 | 主要函数 |
|------|-----------|------|---------|
| `c3.c` / `c3.h` | WiFi 模块 (ESP32-C3) | UART (AT指令) | `c3_init()` 初始化<br>`c3_wifi_tcp_lead()` 连接WiFi/TCP<br>`c3_wifi_tcp_send()` 发送数据<br>`c3_wifi_tcp_receive()` 接收数据 |

---

## 子板单元（U 系列）

| 文件 | 开发板模块 | 接口 | 主要函数 |
|------|-----------|------|---------|
| `u1.c` / `u1.h` | 板载 RUN LED + 软件延时 + Timer0 | GPIO / TIMER | `u1_led_init/on/off/toggle()` LED控制<br>`u1_delay_ms()` 毫秒延时<br>`u1_timer0_init()` 定时器初始化(用于LED闪烁) |

---

## 基础服务

| 文件 | 功能 |
|------|------|
| `i2c.c` / `i2c.h` | I2C 总线初始化、从机检测、寄存器读写 |
| `uart.c` / `uart.h` | 串口初始化与数据收发 |
| `delay.c` / `delay.h` | 延时函数（`delay_ms`、`delay_us`） |
| `bsp_log.h` | 线程安全日志宏 `PRINTF_LOCKED(...)`，内部用 `xUartMutex` 保护 printf |
| `gd32f4xx_libopt.h` | GD32 标准外设库裁剪配置 |

---

## 硬件连接总览

```
GD32F470ZK
  │
  ├── I2C 总线 ──┬── e1 (数码管+LED, 地址0x60-0x63 / 0xC0-0xC6)
  │              ├── e2 (风扇, 地址0x64-0x67 / 0xC8-0xCE)
  │              ├── e3 (窗帘)
  │              ├── s2 (光照/温湿度/IMU)
  │              ├── s5 (NFC)
  │              ├── s8 (温湿度)
  │              └── s11 (称重)
  │
  ├── UART ──────┬── USART0 → c3 (ESP32-C3 WiFi)
  │              ├── USART1 → printf 调试输出
  │              └── USART5
  │
  ├── GPIO ──────┬── s1 (按键矩阵)
  │              ├── s6 (超声波)
  │              ├── s7 (人体红外)
  │              └── u1 (板载LED, PD15)
  │
  └── TIMER ─────── u1 (Timer0, LED闪烁)
```

---

## 使用说明

1. 所有外设句柄在 `main.c` 的 `BSP_Init()` 中统一初始化
2. I2C 总线访问需持有 `xI2CMutex` 互斥锁
3. 串口打印需使用 `PRINTF_LOCKED()` 宏（持有 `xUartMutex`）
4. 各项目通过 `app_hardware.h` 引用外设句柄
