# IoT Java 本地存储版（无需 MySQL / RDS 权限）

## 为什么改成这个版本

原工程依赖阿里云 RDS MySQL。只要数据库账号没有库权限、没有表权限、没有白名单权限，项目就会出现 `Access denied`、数据查询为空或启动失败。

当前版本已经**完全取消外部数据库依赖**：

- 不连接阿里云 RDS；
- 不需要 MySQL 用户名、密码；
- 不需要 RDS 白名单；
- 不执行 `CREATE DATABASE` / `CREATE TABLE`；
- 浏览器用户也不需要任何数据库权限。

架构变为：

`GD32/设备 -> TCP 1025 -> Spring Boot -> 本地 data/ 文件 -> REST API -> Web 前端`

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
- 设备 TCP：运行 Java 的电脑 IP:1025

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
  "tcpPort": 1025
}
```

其中 `databaseRequired=false` 表示此版本不需要数据库权限。

## 设备上报格式

```http
POST /device/update HTTP/1.1
Content-Type: application/json
Content-Length: ...

{"project":20,"name":"智能教室","temperature":26.5,"humidity":61,"light":320}
```

每次上报会：

1. 写入本地 `sensor-records.jsonl`；
2. 更新设备会话包数/字节数；
3. 自动检测温度、湿度、报警标志等告警；
4. 前端可立即读取最新值与历史数据。

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

## 已补功能

- 完全移除 RDS/MySQL 权限依赖；
- 本地持久化，重启不丢数据；
- Project ID 0 可正常新增/修改；
- 传感器历史记录返回真实记录 ID；
- 项目新增/删除后首页动态同步；
- 温度/湿度多项目趋势图真正绘制；
- 告警“一键全部处理”不再写死项目 ID；
- TCP 收包数量和字节数统计；
- 命令统计避免重复计数；
- 程序重启后旧 TCP 会话自动标记离线；
- 首页显示“本地存储”真实状态，而不是伪造 MySQL 正常状态。
