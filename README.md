# IoT 本地存储 — GD32F470 + Java Spring Boot

小米 AIoT 教学实验箱（**GD32F470ZK MCU + ESP32-C3 WiFi 模块**）的 TCP 通信项目。

板子通过 ESP32-C3 连接 WiFi，然后以 TCP 客户端身份连接 Java Spring Boot 后端，上报环境传感器数据（温度/湿度/光照），后端将数据持久化到本地文件并提供 Web 仪表盘。

---

## 快速开始

### 后端

```powershell
cd iot-java-local-storage-no-db
java -jar .\target\iot-java-local-storage-1.0.0.jar
```

启动成功后：

- HTTP Web 仪表盘：**http://localhost:8080**
- Health API：**http://localhost:8080/api/system/health**
- TCP 设备监听端口：**8081**

### 烧录固件（Keil）

1. 打开 `embedded\Project\demo.uvprojx`
2. **Project → Rebuild all target files**（必须 Rebuild，Build 可能漏掉已修改文件）
3. **Flash → Download** 烧录
4. 板子自动运行，10~15 秒后 TCP 连接后端

---

## 通信全流程

```
┌─────────────────────────────────────┐          ┌────────────────────────────────────┐
│   GD32F470ZK + ESP32-C3             │          │   Java Spring Boot                  │
│                                     │          │                                     │
│  1. Reset_Handler                   │          │                                     │
│     ├─ VTOR = 0x08000000  [修复]   │          │                                     │
│     ├─ cpsid i (关中断)   [修复]   │          │                                     │
│     ├─ data/bss 拷贝               │          │                                     │
│     ├─ SystemInit                  │          │                                     │
│     │   └─ FMC_WS = 5    [修复]    │          │                                     │
│     └─ main()                      │          │                                     │
│         ├─ BSP_Init()              │          │                                     │
│         │   ├─ USART0 开中断       │          │                                     │
│         │   ├─ USART5 开中断       │          │                                     │
│         │   ├─ TIMER3 开中断       │          │                                     │
│         │   └─ ...                 │          │                                     │
│         ├─ AppTaskCreate()         │          │                                     │
│         │   ├─ 环境采集任务        │          │                                     │
│         │   ├─ WiFi+TCP任务        │          │                                     │
│         │   ├─ 显示任务            │          │                                     │
│         │   └─ 控制任务            │          │                                     │
│         └─ vTaskStartScheduler()  │          │                                     │
│                                     │          │                                     │
│  2. WiFi + TCP 连接                 │          │                                     │
│     ├─ AT+CWMODE=1                 │          │                                     │
│     ├─ AT+CWJAP="yuwei70",...      │          │                                     │
│     ├─ AT+CIPMUX=0                 │          │                                     │
│     ├─ AT+CIPSTART="TCP",          │  TCP 8081  │  TcpDeviceServer                    │
│     │    "192.168.43.10",8081 ─────┼─────────▶│  (Netty ServerBootstrap)           │
│     └─ TCP 已连接 ✓                │          │                                     │
│                                     │          │                                     │
│  3. 环境数据上报                    │          │                                     │
│     ├─ 读取 DHT11 (温湿度)         │  JSON/TCP │  SensorService.receive()            │
│     ├─ 读取 BH1750 (光照)          │─────────▶│  → LocalStorageSupport 持久化       │
│     ├─ 构造 JSON 帧                │          │                                     │
│     └─ TCP 发送                    │          │                                     │
│                                     │          │                                     │
│                                     │  HTTP    │  GET /api/system/health             │
│                                     │◀─────────│  → onlineDevices=1                 │
│                                     │          │  → sensorRecords=N                  │
│                                     │          │                                     │
└─────────────────────────────────────┘          └────────────────────────────────────┘
```

### 时序详解

| 步骤 | 板子动作 | 后端动作 |
|------|---------|---------|
| 1 | 上电 Reset | - |
| 2 | `Reset_Handler` 设 VTOR → 关中断 → 拷贝 data/bss → `SystemInit`（200MHz PLL + Flash WS=5） | - |
| 3 | `BSP_Init()` 初始化 GPIO/USART/SPI/I2C/TIMER | - |
| 4 | 创建 4 个 FreeRTOS 任务，启动调度器 | - |
| 5 | `c3_init()` → ESP32-C3 AT 自检 | - |
| 6 | AT+CWJAP 连 WiFi `yuwei70` | - |
| 7 | AT+CIPSTART TCP→`192.168.43.10:8081` | `TcpDeviceServer` 接受连接，标记设备 online |
| 8 | 环境采集任务定时读 DHT11/BH1750 | - |
| 9 | 构造 JSON → TCP 发送 | `SensorService` 存入 `data/sensors.json` |
| 10 | - | Web 仪表盘实时展示 |

---

## 硬件连接

| 模块 | 引脚 | 说明 |
|------|------|------|
| ESP32-C3 WiFi | USART5 (PC6=TX, PC7=RX) | 115200 波特率，AT 指令 |
| DHT11 温湿度 | GPIO | 单总线 |
| BH1750 光照 | I2C | 地址 0x23 |
| USB-TTL 调试 | USART0 (PA9=TX) | 115200，PRINTF_LOCKED 输出 |
| CMSIS-DAP | SWD | 烧录 + 调试，OpenOCD 100kHz |

---

## 关键配置

| 配置项 | 文件 | 值 |
|--------|------|-----|
| WiFi SSID | `embedded/BSP/inc/c3.h` | `yuwei70` |
| WiFi 密码 | `embedded/BSP/inc/c3.h` | `yuwei123` |
| TCP 服务器 IP | `embedded/BSP/inc/c3.h` | `192.168.43.10` |
| TCP 服务器端口 | `embedded/BSP/inc/c3.h` | **8081** |
| HTTP 端口 | `iot-java-local-storage-no-db/src/main/resources/application.yml` | `8080` |
| TCP 监听端口 | `iot-java-local-storage-no-db/src/main/resources/application.yml` | **8081** |
| MCU 时钟 | `embedded/CMSIS/gd32f4xx/src/system_gd32f4xx.c` | 200MHz (PLL, 25MHz HXTAL × 400/25/2) |
| Flash 等待周期 | 同上 | **5** [修复] |
| FreeRTOS tick | `embedded/Application/src/FreeRTOSConfig.h` | 200MHz, 1ms |

---

## 修复记录（HardFault 根因）

板子最初上电即 HardFault，无法进入 WiFi/TCP 流程。以下是排查到的 6 个根因及修复：

### 1. VTOR 未设置 → 向量表跳飞

**文件**：`embedded/Startup/startup_gd32f450_470.s`

GD32F470 复位后 VTOR=0，指向 BootROM 而非 Flash (0x08000000)。第一个中断（SysTick/USART/TIMER）会跳到错误的向量表，直接 HardFault。

**修复**：`Reset_Handler` 最开始写 `VTOR = 0x08000000`。

### 2. Flash 等待周期缺失 → BusFault

**文件**：`embedded/CMSIS/gd32f4xx/src/system_gd32f4xx.c`

200MHz 下 CPU 从 Flash 取指令必须插入等待周期。GD32F4 Flash 访问时间 120MHz 下为 0 wait state，但超过 120MHz 必须递增。

**修复**：PLL 配置前写 `FMC_WS = WS_WSCNT_5`。

### 3. USART1 中断开启但无 handler

**文件**：`embedded/BSP/src/uart.c`

`BSP_Init()` 里 `uart_init(USART1)` 同时开了 NVIC 和 USART_INT_RBNE 中断，但原代码只有 `Usart1_Receive_Callback()` 没有 IRQ 包装函数。

**修复**：添加 `void USART1_IRQHandler(void) { Usart1_Receive_Callback(); }`。

### 4. FreeRTOS configCPU_CLOCK_HZ 不匹配

**文件**：`embedded/Application/src/FreeRTOSConfig.h`

原配置 240MHz，实际 PLL 输出 200MHz。SysTick 每 1ms 产生一次 tick，但 FreeRTOS 按 240MHz 计算定时器重载值，实际 tick 周期偏差 1.2 倍。AT 命令 500ms 超时实际只有 ~417ms，ESP32-C3 来不及响应。

**修复**：`configCPU_CLOCK_HZ = 200000000UL`。

### 5. HardFault_Handler 改为死循环 → 防止栈破坏后继续执行

**文件**：`embedded/Startup/startup_gd32f450_470.s` + GNU `.S` 启动文件

原 Default_Handler 是 `bx lr`（返回），对于未处理中断这会跳回任意地址再次 HardFault，形成无限递归。

**修复**：HardFault_Handler 保存 r4-r7/ICSR/CFSR/HFSR/BFAR/MMFAR 到全局 `fault_info`（RAM 0x20000060），然后死循环 `FaultLoop: b FaultLoop`。Default_Handler 统一为 `b .`。

### 6. GNU `.S` 启动文件被 Keil `.s` 覆盖

Keil 会用 ARMCC 格式的 `.s` 文件覆盖 GNU 格式的 `.S`，导致 GCC 编译失败。Keil 的 Build (F7) 只编译有变化的文件，Rebuild All (Project → Rebuild all target files) 会强制重编所有文件。

**经验**：
- 用 Keil 烧录前**必须 Rebuild All**，不要 Build
- GCC `.S` 启动文件和 Keil `.s` 启动文件是两套，不能混用

---

## 串口查看（调试用）

板子有两路串口输出：

| 串口 | 引脚 | 波特率 | 用途 |
|------|------|--------|------|
| USART0 | PA9(TX), PA10(RX) | 115200 | PRINTF_LOCKED 调试输出 |
| USART5 | PC6(TX), PC7(RX) | 115200 | ESP32-C3 AT 指令 |

**查看方法**：
1. USB-TTL 转接器接 **PA9 → TTL RX**，**GND → GND**
2. 插电脑后出 COM 口
3. 用 PuTTY / SSCOM / SecureCRT 打开，参数 `115200 8N1`

---

## 后端 API

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/system/health` | GET | 系统状态，含 onlineDevices / sensorRecords / tcpPort |
| `/api/devices` | GET | 在线设备列表 |
| `/api/sensors/recent` | GET | 最近传感器记录 |

TCP 设备协议：板子以 `\r\n\r\n` 为帧结束符发送 JSON，后端解析后存入 `data/sensors.json`。

---

## 目录结构

```
iot-java-local-storage-no-db-fixed/
├── embedded/                         # 固件
│   ├── CMSIS/gd32f4xx/               # GD32F470 CMSIS 头文件 + system_gd32f4xx.c [修复]
│   ├── Startup/startup_gd32f450_470.s  # Keil 启动文件
│   ├── BSP/
│   │   ├── inc/c3.h                  # ESP32-C3 配置（SSID/IP/端口） [修复]
│   │   └── src/uart.c                # 串口驱动 + USART1_IRQHandler [修复]
│   ├── Application/src/
│   │   ├── FreeRTOSConfig.h          # FreeRTOS 配置 [修复]
│   │   ├── main.c                    # BSP_Init → AppTaskCreate → vTaskStartScheduler
│   │   └── Project_classroom/        # 课堂项目（环境采集 + 4 任务）
│   ├── Project/demo.uvprojx          # Keil 工程
│   ├── Peripherals/                  # GD32F4 HAL
│   └── Source/                       # FreeRTOS V10.3.1
├── iot-java-local-storage-no-db/     # 后端
│   └── src/main/
│       ├── java/com/example/iot/
│       │   ├── tcp/TcpDeviceServer.java     # TCP 监听 8081 [修复]
│       │   ├── controller/SystemController.java  # Health API [修复]
│       │   └── ...
│       └── resources/
│           ├── application.yml             # server.port=8080, iot.tcp.port=8081 [修复]
│           └── static/index.html           # Web 仪表盘
└── README.md
```
