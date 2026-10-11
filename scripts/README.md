# Mock 传感器数据 · 使用说明

通过 TCP 8081 向 Java 后端发送模拟的 GD32/ESP32-C3 传感器上报数据，**无需硬件板子即可测试后端和前端**。

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

## 快速上手

开第二个 CMD 窗口：

```cmd
D:
cd D:\iot-java-local-storage-no-db-fixed
python .\scripts\mock_sensor_data.py
```

默认行为：发 10 条随机数据，每条间隔 3 秒，模拟智能教室（projectId=20）。

输出示例：

```
==================================================
 Mock Sensor Data -> TCP 127.0.0.1:8081
 Project : #20 (Classroom)
 Mode    : 10 records, 3s interval
==================================================
[15:28:54] OK #1  temp=26.6 humi=59.9 light=107 pir=1
[15:28:57] OK #2  temp=27.2 humi=87.1 light=139 pir=1
[15:29:00] OK #3  temp=28.5 humi=81.6 light=261 pir=0
...
Done. Sent 10 records.
```

---

## 所有参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `--server` | `127.0.0.1` | 后端 TCP 服务器地址 |
| `--port` | `8081` | 后端 TCP 端口 |
| `--project-id` | `20` | 项目 ID（0=通用, 20=智能教室, 15=智能窗户...） |
| `--project-name` | `Classroom` | 项目名称 |
| `--count` | `10` | 发送次数（`--continuous` 模式下忽略） |
| `--interval` | `3` | 发送间隔（秒） |
| `--continuous` | 关闭 | 开启后无限循环，Ctrl+C 停止 |

---

## 常用场景

### 场景 1：快速测后端是否能收数据

```cmd
python .\scripts\mock_sensor_data.py --count 3 --interval 1
```

### 场景 2：持续给前端仪表盘"喂数据"（演示用）

```cmd
python .\scripts\mock_sensor_data.py --continuous --interval 2
```

此时浏览器打开 http://localhost:8080，仪表盘数据每 2 秒跳动一次，通信日志面板实时显示收发记录。

### 场景 3：模拟多个不同项目同时上报

开多个 CMD 窗口，每个跑不同项目：

```cmd
REM 窗口 1：智能教室
python .\scripts\mock_sensor_data.py --continuous --project-id 20 --project-name Classroom --interval 3

REM 窗口 2：智能窗户
python .\scripts\mock_sensor_data.py --continuous --project-id 15 --project-name Window --interval 4

REM 窗口 3：智能宠物屋
python .\scripts\mock_sensor_data.py --continuous --project-id 13 --project-name Pet --interval 5
```

### 场景 4：后端在另一台机器

```cmd
python .\scripts\mock_sensor_data.py --server 192.168.43.10 --port 8081 --continuous
```

### 场景 5：构造特定告警场景

湿度过高告警（让 humidity 一直 88+）：

```python
# 在 mock_sensor_data.py 的 send_once 函数里临时改：
humi = round(85.0 + random.random() * 10.0, 1)   # 85.0 ~ 95.0
```

温度告警同理，改 `temp` 的范围即可。

---

## 后端怎么处理 Mock 数据

Mock 脚本发的是**裸 TCP 裸 JSON**，跟板子 ESP32-C3 发的格式完全一样：

```json
{"project":20,"name":"Classroom","temperature":26.6,"humidity":59.9,"light":107,"pir_detected":1}
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

后端**不区分**数据来自真实板子还是 mock 脚本——都会入库、触发告警、刷新在线状态。所以 mock 客户端也会被计入 `onlineDevices`。

---

## Mock 数据的随机范围

| 字段 | 范围 | 物理含义 |
|------|------|----------|
| `temperature` | 20.0 ~ 30.0 °C | 室内温度 |
| `humidity` | 40.0 ~ 90.0 %RH | 相对湿度 |
| `light` | 50 ~ 1000 lux | 照度 |
| `pir_detected` | 0 或 1 | 红外人体检测（0=无人, 1=有人） |

如需特定范围，改 `mock_sensor_data.py` 中 `send_once()` 里对应的 `random.random()` / `random.randint()` 即可。

---

## 验证数据确实入库

### 方法 1：看前端

浏览器打开 http://localhost:8080 → 仪表盘实时数据 + 通信日志面板都能看到。

### 方法 2：调 API

```cmd
REM 健康检查
curl http://localhost:8080/api/system/health

REM 最新 5 条记录
curl http://localhost:8080/api/sensor/20/history?limit=5

REM 通信日志
curl http://localhost:8080/api/comm-logs?limit=10
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

Mock 脚本每发一条就关连接，后端在线判断有 **90 秒宽限期**。只要 90 秒内有新数据上报，前端就显示在线。`--continuous` 模式下没问题。

### Q: 想清空 mock 产生的数据

直接删文件：

```cmd
del D:\iot-java-local-storage-no-db-fixed\iot-java-local-storage-no-db\data\sensor-records.jsonl
```

后端不运行时删除最安全。或者调 API：

```cmd
curl http://localhost:8080/api/comm-logs/clear
```

（注：当前没有清空 sensor-records 的 API，直接删文件即可。）

### Q: 板子和 mock 同时用会冲突吗

不会。板子和 mock 会被当成**两个不同的在线设备**（各自的 TCP socket 独立注册）。但 `onlineDevices` 计数会包含两者，前端只能看到合并后的数据。

---

## 文件索引

| 文件 | 说明 |
|------|------|
| `scripts/mock_sensor_data.py` | **主脚本**（Python，稳定可靠） |
| `scripts/send_to_board.ps1` | PowerShell，通过后端 `/api/tcp-send` 往板子写 JSON |
| `scripts/tcp_test_client.ps1` | 裸 TCP 客户端，发固定文本 |
| `scripts/tcp_test_json.ps1` | HTTP POST JSON 模拟板子上报 |
| `docs/GD32数据解析说明.md` | 后端解析链路详细文档 |
