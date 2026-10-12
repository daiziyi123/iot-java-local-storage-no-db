# IoT Java 本地存储版（无需 MySQL / RDS 权限）

## 项目简介

本项目是一个**物联网（IoT）端到端系统**，由**嵌入式硬件端**和**Java 后端服务**两部分组成，实现"传感器数据采集 → TCP 上报 → 本地持久化存储 → Web 可视化展示 → 远程下发控制命令"的完整闭环。

- **嵌入式端**：基于 **GD32F470ZK（Cortex-M4）+ FreeRTOS** 实时操作系统，通过 I2C/UART/GPIO 采集温湿度、光照、人体红外等传感器数据，经 ESP32-C3 WiFi 模块以 TCP 协议上报给后端，并接收后端下发的控制命令（切换模式、调节风扇/灯光/窗帘等）。内置 7 个场景项目（智能教室、无人机、农场、宠物、垃圾桶、窗户等），通过宏切换。
- **后端服务**：基于 **Spring Boot** 框架，使用**本地 JSON 文件持久化**（无需 MySQL/RDS 数据库），提供 REST API 和 TCP 设备服务（端口 1025），内置静态前端页面实现数据可视化与设备控制。
- **通信协议**：开发板与后端通过 TCP Socket 通信，数据以 JSON 格式交互。

适用于物联网教学、原型验证及对数据库无依赖的轻量级部署场景。

## 目录结构

```
iot-java-local-storage-no-db/        # 本仓库
├── iot-java-local-storage-no-db/   # Java 后端（Spring Boot）
│   ├── src/main/java/com/example/iot/
│   │   ├── controller/       # REST API 控制层
│   │   ├── service/          # 业务逻辑
│   │   ├── storage/          # 本地文件持久化
│   │   ├── tcp/              # TCP 设备服务器
│   │   └── model/            # 数据模型
│   ├── src/main/resources/
│   │   ├── static/           # 前端 HTML/CSS/JS（改完刷新即生效，无需重启）
│   │   └── application.yml
│   ├── pom.xml
│   └── README.md
├── embedded/                # 嵌入式硬件工程（本仓库新增）
│   ├── Application/          # 用户应用层（WiFi/TCP、传感器、控制逻辑）
│   ├── BSP/                  # 板级支持包（UART/I2C/GPIO）
│   ├── CMSIS/                # ARM CMSIS
│   ├── Peripherals/          # GD32F4xx 外设驱动
│   ├── Project/              # Keil / EIDE 工程文件
│   ├── Source/               # FreeRTOS
│   ├── Startup/              # 启动文件
│   └── tcp server2.py        # Python 原型服务器（开发调试用）
└── .gitignore
```

## 本地数据文件

程序第一次运行会自动创建 `./data`：

- `projects.json`：项目配置
- `sensor-records.jsonl`：传感器历史数据（追加写，适合高频上报）
- `alerts.json`：告警记录
- `device-sessions.json`：TCP 设备会话

因此 Java 程序重启以后数据仍然保留。

可以通过环境变量修改数据目录：

```bash
IOT_STORAGE_DIR=/your/path/data
```

默认最多加载最近 5000 条传感器记录，可通过：

```bash
IOT_MAX_SENSOR_RECORDS=10000
```

修改。

## 启动

### 方式 1：直接运行已打包 JAR

```bash
java -jar target/iot-java-local-storage-1.0.0.jar
```

### 方式 2：源码运行

需要 JDK 17+ 与 Maven：

```bash
mvn spring-boot:run
```

打开：

- Web：http://127.0.0.1:8080
- 系统健康检查：http://127.0.0.1:8080/api/system/health
- 设备 TCP：运行 Java 的电脑 IP:8081

## 健康检查

访问：

```text
GET /api/system/health
```

正常示例：

```json
{
  "service": "ok",
  "databaseRequired": false,
  "database": "disabled",
  "storage": "local-file",
  "storageWritable": true,
  "tcpPort": 8081
}
```

其中 `databaseRequired=false` 表示此版本不需要数据库权限。

## 设备上报格式

嵌入式设备向 TCP 8081 发送裸 JSON（每 5 秒一次）：  

```json
{"project": 20, "name": "智能教室", "temperature": 26.5, "humidity": 61, "light": 320}
```

Java 后端同时兼容 HTTP POST 风格（`POST /device/update`），两种格式都能正确解析。

每次上报会：

1. 写入本地 `sensor-records.jsonl`；
2. 更新设备会话包数/字节数；
3. 自动检测温度、湿度、报警标志等告警；
4. 前端可立即读取最新值与历史数据。

## 服务器下发命令

Java 后端向设备下发单字符 ASCII 命令，有两种方式：

- **实时直接发**：TCP 长连接存活时，`socket.write('A')` 立刻发送
- **随响应一起发**：命令排队等待，设备下次上报时随 HTTP 200 响应体一起取走：`{"status":"OK","cmd":"M"}`

嵌入式端收到数据后取第一个字符（剥离 ESP32-C3 的 `+IPD,len:` 前缀）作为命令，通过 FreeRTOS 队列分发给控制任务。

## 主要 API

- `GET /api/system/health`：服务/本地存储状态
- `GET /api/project`：项目列表
- `POST /api/project`：新增/编辑项目（支持 Project ID = 0）
- `DELETE /api/project/{id}`：删除项目
- `GET /api/sensor/{pid}`：最新传感器数据
- `GET /api/sensor/{pid}/history?limit=20`：历史数据
- `POST /api/command/{pid}`：向在线设备发送单字符命令
- `GET /api/device`：设备会话
- `GET /api/alert`：告警列表
- `POST /api/alert/resolve-all`：全部告警标记已处理

## 嵌入式端（embedded/）

### 硬件

- 主控：GD32F4xx（Cortex-M4）
- 实时系统：FreeRTOS
- WiFi 模块：ESP32-C3（AT 指令模式）
- 通信协议：TCP 客户端，连接服务器 1025 端口

### 构建

推荐使用 EIDE（Embedded IDE，VS Code 插件），工程文件位于 `embedded/Project/`。

也可用 Keil MDK，工程文件为 `embedded/Project/demo.uvprojx`。

构建产物（`.hex` / `.axf` / `.o` / `.map` 等）已在 `.gitignore` 中忽略，重新编译即得。

### 已实现项目

| Project ID | 名称 | 说明 |
| :--------: | ---- | ---- |
| 0 | 基础 IoT 控制 | 温度/湿度/光照上报，LED/风扇/窗帘控制 |
| 13 | 智能宠物屋 | 距离、进窝检测、喂食提醒 |
| 15 | 智能窗户 | 温湿度/光照/PIR，窗帘自动调节 |
| 16 | 智能垃圾桶 | 距离/容量/开盖/压缩 |
| 17 | 智能农场 | 光照/温湿度/作物高度，灌溉/遮阳 |
| 20 | 智能教室 | 温湿度/光照/PIR，风扇/LED/窗帘/模式切换 |
| 25 | 智能无人机 | 高度/温湿度/电量，起飞/降落/悬停模式 |

### Python 原型服务器

`embedded/tcp server2.py` 是开发阶段用的原型验证服务器，提供：

- TCP 1025：收设备上报、发控制命令
- Flask 5000：各项目的控制页面

生产环境请使用 Java 后端（本仓库主目录），两者协议完全一致。


