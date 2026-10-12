# FreeRTOS 使用说明

## 一、FreeRTOS 简介

FreeRTOS 是一个开源的实时操作系统内核（RTOS Kernel），适用于微控制器和小型微处理器。

- **创始人**：Richard Barry，于 2003 年创建
- **原维护方**：Real Time Engineers Ltd.（Richard Barry 的公司）
- **当前维护方**：Amazon Web Services (AWS)，2017 年接手项目 stewardship，Richard Barry 作为 AWS 团队成员继续参与开发
- **许可证**：MIT 许可证（自 V10 起，之前为修改版 GPLv2）
- **本项目版本**：FreeRTOS Kernel V11.1.0（属于 FreeRTOS 202406.00 LTS 版本）

---

## 二、本项目中的 FreeRTOS 配置

配置文件：[Application/src/FreeRTOSConfig.h](../Application/src/FreeRTOSConfig.h)

### 关键配置项

| 配置宏 | 值 | 说明 |
|--------|-----|------|
| `configCPU_CLOCK_HZ` | 200000000 | CPU 主频 200MHz，必须与实际系统时钟一致 |
| `configTICK_RATE_HZ` | 1000 | 系统滴答频率 1000Hz，即 1ms 一个 tick |
| `configMAX_PRIORITIES` | 8 | 最大优先级数（0~7，数字越大优先级越高） |
| `configMINIMAL_STACK_SIZE` | 128 | 任务最小栈大小（单位：word，即 128×4=512 字节） |
| `configTOTAL_HEAP_SIZE` | 16×1024 | FreeRTOS 堆大小 16KB，用于任务栈、队列、信号量等动态分配 |
| `configUSE_PREEMPTION` | 1 | 启用抢占式调度 |
| `configUSE_TIME_SLICING` | 1 | 启用时间片轮转（同优先级任务轮流执行） |
| `configUSE_MUTEXES` | 1 | 启用互斥锁 |
| `configUSE_COUNTING_SEMAPHORES` | 1 | 启用计数信号量 |
| `configIDLE_SHOULD_YIELD` | 1 | 空闲任务让出 CPU 给同优先级任务 |

### 启用的 API

| API | 说明 |
|-----|------|
| `vTaskDelay` | 相对延时 |
| `vTaskDelayUntil` | 绝对周期延时 |
| `xTaskGetSchedulerState` | 获取调度器状态 |
| `xSemaphoreGetMutexHolder` | 获取互斥锁持有者 |

### 中断映射

```c
#define xPortPendSVHandler  PendSV_Handler   // 任务切换
#define vPortSVCHandler     SVC_Handler      // 系统服务调用
#define xPortSysTickHandler SysTick_Handler  // 系统滴答
```

---

## 三、本工程中的 FreeRTOS 集成

### 目录结构

```
embedded/
├── Source/                          ← FreeRTOS 内核源码
│   ├── tasks.c                      ← 任务调度核心
│   ├── queue.c                      ← 队列/信号量/互斥锁
│   ├── list.c                       ← 链表数据结构
│   ├── timers.c                     ← 软件定时器
│   ├── event_groups.c               ← 事件组
│   ├── croutine.c                   ← 协程（可选）
│   ├── include/                     ← 内核头文件
│   └── portable/
│       ├── RVDS/ARM_CM4F/port.c     ← Cortex-M4 移植层（Keil AC5）
│       └── MemMang/heap_4.c         ← 内存管理（heap_4 方案）
└── Application/src/
    ├── FreeRTOSConfig.h             ← 本项目的 FreeRTOS 配置
    └── project_classroom/           ← 应用层任务
        ├── app_project.c            ← 任务创建
        ├── app_sensor.c             ← 传感器任务
        ├── app_control.c            ← 控制任务
        ├── app_wifi.c               ← WiFi 任务
        └── app_display.c            ← 显示任务
```

### Keil 工程配置（demo.uvprojx）

1. **添加源文件**：tasks.c、queue.c、list.c、timers.c、event_groups.c、croutine.c、port.c、heap_4.c
2. **头文件包含路径**：
   - `../Source/include`
   - `../Source/portable/RVDS/ARM_CM4F`
3. **编译器**：ARMCC AC5（对应 `portable/RVDS/ARM_CM4F`）

---

## 四、核心 API 使用（结合本项目代码）

### 1. 创建任务

```c
// 函数原型
BaseType_t xTaskCreate(
    TaskFunction_t pvTaskCode,        // 任务函数指针
    const char * const pcName,        // 任务名（调试用）
    configSTACK_DEPTH_TYPE usStackDepth, // 栈大小（单位 word）
    void *pvParameters,               // 传给任务的参数
    UBaseType_t uxPriority,           // 优先级（0最低，configMAX_PRIORITIES-1最高）
    TaskHandle_t *pxCreatedTask       // 任务句柄（可传 NULL）
);
```

**本项目示例**（[app_project.c](../Application/src/project_classroom/app_project.c)）：

```c
xTaskCreate(vTaskSensorRead, "ClsSensor", 512, NULL, 4, NULL);
xTaskCreate(vTaskControl,    "ClsCtrl",   512, NULL, 3, NULL);
xTaskCreate(vTaskWifiAT,     "ClsWifi",   1024, NULL, 2, NULL);
xTaskCreate(vTaskDisplay,    "ClsDisp",   256, NULL, 1, NULL);
```

### 2. 创建互斥锁

```c
// 创建
SemaphoreHandle_t xSemaphoreCreateMutex(void);

// 加锁（获取）
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);

// 解锁（释放）
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);
```

**本项目示例**：

```c
// 创建
xUartMutex = xSemaphoreCreateMutex();
xI2CMutex  = xSemaphoreCreateMutex();

// 使用（app_sensor.c）
if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
    // 读取 I2C 传感器...
    xSemaphoreGive(xI2CMutex);
}
```

### 3. 创建和使用队列

```c
// 创建
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);

// 发送
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait);

// 接收
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait);
```

**本项目示例**（WiFi 任务 → 控制任务传递命令）：

```c
// 创建（app_project.c）
xClassroomCommandQueue = xQueueCreate(8, sizeof(char));

// 发送（app_wifi.c）
xQueueSend(xClassroomCommandQueue, &cmd, pdMS_TO_TICKS(100));

// 接收（app_control.c）
if (xQueueReceive(xClassroomCommandQueue, &cmd, pdMS_TO_TICKS(200)) == pdTRUE) {
    // 处理命令...
}
```

### 4. 任务延时

```c
// 相对延时：从调用时刻开始延时指定时间
void vTaskDelay(TickType_t xTicksToDelay);

// 绝对周期延时：保证任务以固定周期执行
void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement);

// 毫秒转 tick 宏
pdMS_TO_TICKS(ms)
```

**本项目示例**：

```c
// 精确周期延时（app_sensor.c，保证每 200ms 采集一次）
TickType_t lastWake = xTaskGetTickCount();
for (;;) {
    // 采集传感器...
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
}

// 普通延时（app_wifi.c）
vTaskDelay(pdMS_TO_TICKS(100));
```

### 5. 启动调度器

```c
void vTaskStartScheduler(void);
```

**本项目示例**（[main.c](../Application/src/main.c)）：

```c
int main(void)
{
    BSP_Init();           // 硬件初始化
    AppTaskCreate();      // 创建互斥锁和任务
    vTaskStartScheduler(); // 启动调度器，正常不返回
    while (1);            // 兜底死循环
}
```

---

## 五、本项目中的任务清单

| 任务名 | 函数 | 优先级 | 栈大小 | 周期 | 职责 |
|--------|------|--------|--------|------|------|
| ClsSensor | vTaskSensorRead | 4 | 512 | 200ms | 采集温湿度/光照/红外/按键 |
| ClsCtrl | vTaskControl | 3 | 512 | 200ms | 命令处理、自动控制、能耗统计 |
| ClsWifi | vTaskWifiAT | 2 | 1024 | 100ms | TCP通信、数据上报、命令接收 |
| ClsDisp | vTaskDisplay | 1 | 256 | 300ms | 数码管显示、LED状态指示 |

---

## 六、同步机制总览

| 对象 | 类型 | 保护/传递内容 | 创建位置 |
|------|------|--------------|---------|
| `xUartMutex` | 互斥锁 | printf 串口输出 | main.c |
| `xI2CMutex` | 互斥锁 | I2C 总线访问 | main.c |
| `xClassroomMutex` | 互斥锁 | g_classroomData 共享数据 | app_project.c |
| `xClassroomCommandQueue` | 队列 | WiFi→控制的命令(char) | app_project.c |

---

## 七、注意事项

1. **configCPU_CLOCK_HZ 必须与实际系统时钟一致**，否则 tick 不准，所有延时和超时都会出错
2. **互斥锁必须成对使用**：`xSemaphoreTake` 后必须 `xSemaphoreGive`，否则会死锁
3. **任务内必须有阻塞调用**（如 `vTaskDelay`），否则高优先级任务会饿死低优先级任务
4. **中断中不能调用 `xSemaphoreTake`**，应使用 `FromISR` 版本的 API
5. **堆大小 `configTOTAL_HEAP_SIZE`** 需要足够容纳所有任务栈、队列、信号量，本项目设为 16KB
6. **优先级不要设满**：本项目最高用 4，留有余量
