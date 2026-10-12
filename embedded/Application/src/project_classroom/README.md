# 智能教室项目实现说明

本文档说明 `project_classroom` 目录下智能教室系统的实现方式、使用的模块、以及与开发板的物理连接通信。

---

## 一、项目概述

智能教室系统基于 GD32F470ZK + FreeRTOS，实现对教室环境的**自动感知、智能调控、远程监控**。

核心功能：
- 自动采集温度、湿度、光照、人体存在等环境数据
- 根据环境数据自动调节风扇、灯光、窗帘
- 支持三种工作模式（正常/节能/演示）
- 支持远程手动控制（通过 WiFi TCP）
- 定时上报环境数据与控制状态到上位机
- 本地数码管显示 + LED 状态指示

---

## 二、实现架构（FreeRTOS 多任务）

系统通过 [app_project.c](file:///e:/gd32/iot-java-local-storage-no-db/embedded/Application/src/project_classroom/app_project.c) 创建 4 个并发任务：

| 任务 | 函数 | 优先级 | 栈大小 | 周期 | 职责 |
|------|------|--------|--------|------|------|
| 传感器采集 | `vTaskSensorRead` | 4（最高） | 512 | 200ms | 读取温湿度/光照/红外/按键，写入共享数据 |
| 控制逻辑 | `vTaskControl` | 3 | 512 | 200ms | 处理命令、自动调控执行器、能耗统计 |
| WiFi通信 | `vTaskWifiAT` | 2 | 1024 | 100ms | TCP连接维护、接收命令、定时上报数据 |
| 显示 | `vTaskDisplay` | 1（最低） | 256 | 300ms | 数码管轮显、LED模式指示 |

### 任务创建入口

```c
void app_project_create_tasks_classroom(void)
{
    xClassroomMutex       = xSemaphoreCreateMutex();        // 共享数据互斥锁
    xClassroomCommandQueue = xQueueCreate(8, sizeof(char));  // 命令队列

    xTaskCreate(vTaskSensorRead, "ClsSensor", 512, NULL, 4, NULL);
    xTaskCreate(vTaskControl,    "ClsCtrl",   512, NULL, 3, NULL);
    xTaskCreate(vTaskWifiAT,     "ClsWifi",   1024, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay,    "ClsDisp",   256, NULL, 1, NULL);
}
```

---

## 三、共享数据结构

[classroom_types.h](file:///e:/gd32/iot-java-local-storage-no-db/embedded/Application/src/project_classroom/classroom_types.h) 定义 `ClassroomData_t`：

```c
typedef struct {
    // 传感器数据
    float temperature;        // 温度 (°C)
    float humidity;           // 湿度 (%)
    float light;              // 照度 (lux)
    unsigned char pir_detected; // 人体红外 (0/1)
    uint8_t key_pressed;      // 按键触发标志

    // 系统状态
    uint8_t mode;             // 0=正常, 1=节能, 2=演示
    uint8_t fan_speed;        // 风扇转速 0~100
    uint8_t led_brightness;   // LED亮度 0~100
    uint8_t curtain_pos;      // 窗帘位置 0~100

    // 能耗统计
    uint32_t occupancy_sec;   // 有人累计秒数
    uint32_t total_sec;       // 运行总秒数
    uint32_t fan_runtime;     // 风扇运行秒数
    uint32_t light_runtime;   // 灯光运行秒数
    bool manual_lock;         // 手动锁定标志
} ClassroomData_t;
```

所有任务通过 `g_classroomData` 全局结构体共享数据，用 `xClassroomMutex` 互斥锁保护。

---

## 四、使用的硬件模块

### 传感器（输入）

| 模块 | BSP驱动 | 接口 | 采集数据 | 函数 |
|------|---------|------|---------|------|
| 温湿度传感器 | s8 | I2C | 温度、湿度 | `s8_ths_value_get()` |
| 光照传感器 | s2 | I2C | 照度(lux) | `s2_illuminance_value_get()` |
| 人体红外 | s7 | GPIO | 有人/无人 | `s7_ir_status_get()` |
| 按键 | s1 | GPIO | 按键值 | `s1_key_value_get()` |

### 执行器（输出）

| 模块 | BSP驱动 | 接口 | 控制内容 | 函数 |
|------|---------|------|---------|------|
| RGB LED | e1 | I2C | 三色亮度 | `e1_led_rgb_set(r,g,b)` |
| 数码管 | e1 | I2C | 显示字符串 | `e1_tube_str_set()` |
| 风扇 | e2 | I2C | 转速 0~100% | `e2_fan_speed_set()` |
| 窗帘 | e3 | I2C | 位置 0~100 | `e3_curtain_position_set()` |

### 通信

| 模块 | BSP驱动 | 接口 | 功能 | 函数 |
|------|---------|------|------|------|
| ESP32-C3 WiFi | c3 | UART(AT指令) | 连WiFi、建TCP、收发数据 | `c3_wifi_tcp_lead/send/receive` |

---

## 五、物理连接与通信

### 总线连接图

```
GD32F470ZK
  │
  ├─ I2C 总线 ──┬── e1 数码管+RGB LED (PCA9685 + HT16K33)
  │             ├── e2 风扇 (PCA9685 PWM)
  │             ├── e3 窗帘电机
  │             ├── s2 光照传感器
  │             └── s8 温湿度传感器
  │
  ├─ UART0 ──────── c3 ESP32-C3 WiFi 模块 (AT指令)
  ├─ UART1 ──────── printf 调试输出
  │
  ├─ GPIO ──┬── s1 按键矩阵
  │         └── s7 人体红外
  │
  └─ 软件 I2C 互斥锁保护所有 I2C 访问
```

### 通信协议

开发板与上位机通过 **WiFi TCP** 通信，数据格式为 **JSON**：

**上报1 — 环境数据（每7秒）：**
```json
{"project":20,"name":"Classroom",
 "temperature":26.5,"humidity":50.0,"light":320,"pir_detected":1}
```

**上报2 — 控制与统计数据（每7秒，相位偏移3.5秒）：**
```json
{"project":20,"name":"Classroom",
 "mode":0,"fan_speed":50,"led_brightness":80,
 "curtain_pos":50,"usage":75,"fan_runtime":1200,"light_runtime":3600}
```

**接收命令（单字符）：**

| 命令 | 含义 |
|------|------|
| `M` | 切换模式（正常→节能→演示循环） |
| `F` | 远程手动开关风扇（进入手动锁定） |
| `L` | 远程手动开关灯光（进入手动锁定） |
| `O` | 解除手动锁定，恢复自动控制 |

---

## 六、数据流向

```
┌──────────────────────────────────────────────────────────────┐
│                        硬件传感器                              │
│  s8温湿度  s2光照  s7红外  s1按键                              │
└──────────────────────┬───────────────────────────────────────┘
                       │ I2C/GPIO
                       ▼
┌──────────────────────────────────────────────────────────────┐
│  vTaskSensorRead (优先级4, 200ms)                             │
│  加xI2CMutex读传感器 → 加xClassroomMutex写g_classroomData      │
└──────────────────────┬───────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────────────────────┐
│           g_classroomData (xClassroomMutex保护)                │
└──────┬───────────────────┬───────────────────┬────────────────┘
       │                   │                   │
       ▼                   ▼                   ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────────┐
│ vTaskControl │  │ vTaskWifiAT  │  │  vTaskDisplay    │
│ (优先级3)    │  │ (优先级2)    │  │  (优先级1)       │
│              │  │              │  │                  │
│ • 处理命令队列│  │ • 接收远程命令│  │ • 数码管轮显     │
│ • 自动调控   │  │ • 上报JSON   │  │ • LED模式色指示  │
│ • 能耗统计   │  │              │  │                  │
└──────┬───────┘  └──────┬───────┘  └──────────────────┘
       │                 │
       ▼                 ▼
┌──────────────┐  ┌──────────────┐
│ 执行器        │  │ ESP32-C3     │
│ e2风扇/e1LED  │  │ WiFi → 上位机 │
│ e3窗帘       │  │              │
└──────────────┘  └──────────────┘
```

---

## 七、自动控制逻辑（vTaskControl）

### 无人时
- 关闭风扇、灯光、窗帘（强制，除非手动锁定）

### 有人时，按模式调控

**光照控制（LED亮度 + 窗帘位置）：**

| 模式 | 照度 < 300 lux | 照度 300~600 | 照度 > 600 |
|------|---------------|-------------|-----------|
| 正常(0) | LED=80, 窗帘=20 | LED=40, 窗帘=50 | LED=0, 窗帘=80 |
| 节能(1) | LED=50, 窗帘=30 | LED=0, 窗帘=70 | LED=0, 窗帘=70 |
| 演示(2) | LED=100, 窗帘=30 | LED=100, 窗帘=30 | LED=100, 窗帘=30 |

**温度控制（风扇转速）：**

| 温度 | 风扇转速 |
|------|---------|
| > 28°C | 100% |
| 24~28°C | 50% |
| < 24°C | 0% |

节能模式下风扇转速再 × 0.6。

### 能耗统计
每秒累计：`total_sec`（总运行）、`occupancy_sec`（有人时长）、`fan_runtime`（风扇运行）、`light_runtime`（灯光运行）。

使用率 = `occupancy_sec / total_sec × 100%`。

---

## 八、同步机制

| 同步对象 | 保护内容 | 创建位置 |
|---------|---------|---------|
| `xClassroomMutex` | `g_classroomData` 共享数据 | app_project.c |
| `xI2CMutex` | I2C 总线访问 | main.c |
| `xUartMutex` | printf 串口输出 | main.c |
| `xClassroomCommandQueue` | WiFi任务 → 控制任务的命令传递 | app_project.c |

---

## 九、文件清单

| 文件 | 作用 |
|------|------|
| `classroom_types.h` | 数据结构定义 |
| `app_project.c/.h` | 任务创建入口 |
| `app_sensor.c/.h` | 传感器采集任务 |
| `app_control.c/.h` | 控制逻辑任务 |
| `app_wifi.c/.h` | WiFi通信任务 |
| `app_display.c/.h` | 显示任务 |
