# Mock 传感器数据 · 使用说明

通过 TCP 8081 向 Java 后端发送模拟的 GD32/ESP32-C3 传感器上报数据，**无需硬件板子即可测试后端和前端**。支持同时模拟多个项目并发上报。

---

## 前置条件

- Python 3.6+（已验证 3.13.1）
- Java 后端已启动并监听 8081 端口

## 启动后端

开一个 CMD 窗口（别关）：

```cmd
D:
cd D:\iot-java-local-storage-no-db-fixed\iot-java-local-storage-no-db
java -jar target\iot-java-local-storage-1.0.0.jar
```

等看到 `Started IotApplication` 即后端就绪。

---

## 两种运行模式

| 模式 | 触发 | 用途 |
|------|------|------|
| **交互式菜单**（默认） | `python mock_sensor_data.py` | 开多个项目后台线程，同时跑多个 |
| **命令行**（CLI） | `python mock_sensor_data.py --project-id 20 --count 10` | 单个项目，脚本/定时任务用 |

---

## 模式一：交互式菜单（多项目并发）

### 启动

```cmd
D:
cd D:\iot-java-local-storage-no-db-fixed
python .\scripts\mock_sensor_data.py
```

菜单：

```
============================================================
  Mock 多项目模拟器（输入项目ID即启动）
============================================================
  后端     : 127.0.0.1:8081
  时间     : 16:30:00
------------------------------------------------------------
  输入项目ID启动（用逗号分隔可同时启动多个）:
       0 = 基础 IoT 控制         13 = 智能宠物屋
      15 = 智能窗户              16 = 智能垃圾桶
      17 = 智能农场              20 = 智能教室
      25 = 智能无人机
------------------------------------------------------------
  list / l              查看运行中项目
  stop <pid|all>        停止项目（例: stop 20, stop all）
  s                     修改后端地址
  q / quit              退出程序
============================================================
```

### 基本操作

| 输入 | 效果 |
|------|------|
| `20` | 启动智能教室后台线程，每 3 秒发一条 |
| `20,15,13` | 同时启动三个项目（逗号/空格/中文逗号都能分隔） |
| `l` / `list` | 查看当前运行中哪些项目 |
| `stop 20` | 停止项目 20 |
| `stop all` | 停掉所有 |
| `s` | 修改后端地址（会自动重启已有线程） |
| `q` | 全部停止后退出 |

### 多项目并发效果

```
> 20,15,13
  ✓ 启动 #20 (智能教室)  每 3 秒发一次
  ✓ 启动 #15 (智能窗户)  每 3 秒发一次
  ✓ 启动 #13 (智能宠物屋) 每 3 秒发一次

> l
  运行中 3 个项目:
    ✓ #13 智能宠物屋   seq=5  interval=3s
    ✓ #15 智能窗户     seq=5  interval=3s
    ✓ #20 智能教室     seq=5  interval=3s

[16:30:12] #20 OK #  1  t=25.8C h=61.2%  l=530 pir=无人
[16:30:12] #15 OK #  1  t=24.9C h=58.7%  l=480 pir=无人
[16:30:12] #13 OK #  1  t=27.2C h=72.1%  l=612 pir=有人
...

> stop all
  ■ 停止 #13
  ■ 停止 #15
  ■ 停止 #20

> q
再见！
```

每个项目一个独立的 **Python Thread（daemon=True）**，互不干扰。后端 `onlineDevices` 计数会正确显示并发项目总数。

### 停止

- 输入 `stop 20` → 停单个
- 输入 `stop all` → 全停
- 输入 `q` → 全停 + 退出
- 直接关闭 CMD 窗口也可以（daemon 线程自动终止）

---

## 模式二：命令行（单项目）

脚本 / 定时任务用，不进入交互菜单。

### 全部参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `--server` | `127.0.0.1` | 后端 TCP 服务器地址 |
| `--port` | `8081` | 后端 TCP 端口 |
| `--project-id` | `20` | 项目 ID |
| `--interval` | `3` | 发送间隔（秒） |
| `--count` | `0` | 发 N 条后退出（0 = 无限循环） |
| `--continuous` | （自动） | 省略 `--count` 或设 0 即无限循环 |

### 常用命令

```cmd
REM 发 10 条智能教室，间隔 2 秒
python .\scripts\mock_sensor_data.py --project-id 20 --count 10 --interval 2

REM 无限循环发智能窗户
python .\scripts\mock_sensor_data.py --project-id 15 --interval 3

REM 后端在另一台机器
python .\scripts\mock_sensor_data.py --server 192.168.43.10 --project-id 20
```

### 多项目 CLI 方式

开多个 CMD 窗口：

```cmd
REM 窗口 1
python .\scripts\mock_sensor_data.py --project-id 20 --interval 3

REM 窗口 2
python .\scripts\mock_sensor_data.py --project-id 15 --interval 4

REM 窗口 3
python .\scripts\mock_sensor_data.py --project-id 13 --interval 5
```

---

## 项目 ID 映射表

与后端 `TcpDeviceServer.defaultProjectName()` 完全一致：

| ID | 名称 |
|----|------|
| 0 | 基础 IoT 控制 |
| 13 | 智能宠物屋 |
| 15 | 智能窗户 |
| 16 | 智能垃圾桶 |
| 17 | 智能农场 |
| 20 | 智能教室 |
| 25 | 智能无人机 |

---

## Mock 数据的随机范围

| 字段 | 范围 | 物理含义 |
|------|------|----------|
| `temperature` | 20.0 ~ 30.0 °C | 室内温度 |
| `humidity` | 40.0 ~ 90.0 %RH | 相对湿度 |
| `light` | 8 点~18 点: 300~1000 lux<br>其他时间: 0~100 lux | 照度（模拟昼夜） |
| `pir_detected` | 40% 概率为 1 | 红外人体检测 |

如需特定范围（比如构造高温告警），改 `mock_sensor_data.py` 中 `gen_data()` 函数里的 `random.random()` / `random.randint()` 即可。

---

## 后端怎么处理 Mock 数据

Mock 脚本发的是**裸 TCP 裸 JSON**，跟板子 ESP32-C3 发的格式完全一样：

```json
{"project":20,"name":"智能教室","temperature":26.6,"humidity":59.9,"light":530,"pir_detected":0}
```

后端 `TcpDeviceServer` 处理流程：

```
TCP 8081 accept() → handleClient()
  → 读字节到 ByteArrayOutputStream
  → processRequest()
    → Jackson.readValue → Map<String,Object>
    → json.remove("project") / json.remove("name")
    → sensorService.save()  → 写 data/sensor-records.jsonl
    → alertService.detect() → 阈值告警检测
    → connectionManager.touch() → 刷新在线心跳
  → sendHttp(200, {"status":"OK"})
```

后端**不区分**数据来自真实板子还是 mock 脚本——都会入库、触发告警、刷新在线状态。多个 mock 客户端同时连接时，`DeviceConnectionManager.sockets`（`ConcurrentHashMap<Integer, Socket>`）为每个 projectId 维护独立 socket。

---

## 验证数据确实入库

### 方法 1：看前端

浏览器打开 http://localhost:8080 → 仪表盘实时数据 + 通信日志面板都能看到。多个项目同时在线时 `onlineDevices` 计数正确。

### 方法 2：调 API

```cmd
REM 健康检查
curl http://localhost:8080/api/system/health

REM 最新 5 条记录（某项目）
curl http://localhost:8080/api/sensor/20/history?limit=5

REM 通信日志（能看到所有项目的收发记录）
curl http://localhost:8080/api/comm-logs?limit=20
```

### 方法 3：看原始文件

```cmd
type D:\iot-java-local-storage-no-db-fixed\iot-java-local-storage-no-db\data\sensor-records.jsonl | more
```

---

## 常见问题

### Q: Connection refused / 10061

后端没启动或端口不对。确认后端 CMD 窗口里有 `Started IotApplication`，并且没有被关闭。

### Q: Mock 成功入库，但前端显示离线

Mock 脚本每发一条就关连接，后端在线判断有 **90 秒宽限期**。只要 90 秒内有新数据上报，前端就显示在线。交互模式（每 3 秒一条）和 CLI `--continuous` 都没问题。

### Q: 想清空 mock 产生的数据

直接删文件：

```cmd
del D:\iot-java-local-storage-no-db-fixed\iot-java-local-storage-no-db\data\sensor-records.jsonl
```

后端不运行时删除最安全。通信日志可调 API：

```cmd
curl http://localhost:8080/api/comm-logs/clear
```

### Q: 板子和 mock 同时用会冲突吗

不会。板子和 mock 会被当成**两个不同的在线设备**（各自的 TCP socket 独立注册在 `DeviceConnectionManager.sockets` 里，key 都是各自的 projectId）。同一个 projectId 同时有板子和 mock 连的话，后连的会**覆盖**先连的 socket（`put` 操作，旧的被 close）。

### Q: 多个项目的线程安全吗

安全。每个 `MockProjectThread` 是 daemon 线程，用 `threading.Event` 控制停止，发送函数 `send_once()` 里每次新建 socket（短连接），不共享任何状态。

---

## 文件索引

| 文件 | 说明 |
|------|------|
| `scripts/mock_sensor_data.py` | **主脚本**（交互式多项目 + CLI 双模式） |
| `scripts/send_to_board.ps1` | PowerShell，通过后端 `/api/tcp-send` 往板子写 JSON |
| `scripts/tcp_test_client.ps1` | 裸 TCP 客户端，发固定文本 |
| `scripts/tcp_test_json.ps1` | HTTP POST JSON 模拟板子上报 |
| `scripts/mock数据说明.md` | 本文档 |
| `docs/GD32数据解析说明.md` | 后端解析链路详细文档 |
