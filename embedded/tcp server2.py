import socket
import threading
import json
import time
from flask import Flask, render_template_string, request, jsonify
from typing import Optional, Dict

# ========== 全局数据结构 ==========
projects: Dict[int, dict] = {}  # project_id -> {"sensor": {...}, "conn": socket, "name": str}
projects_lock = threading.Lock()

# ========== 预设项目列表（全部已实现）==========
PROJECT_LIST = [
    {"id": 0, "name": "基础 IoT 控制", "has_page": True},
    {"id": 15, "name": "智能窗户", "has_page": True},
    {"id": 20, "name": "智能教室", "has_page": True},
    {"id": 17, "name": "智能农场", "has_page": True},
    {"id": 25, "name": "智能无人机", "has_page": True},
    {"id": 13, "name": "智能宠物屋", "has_page": True},
    {"id": 16, "name": "智能垃圾桶", "has_page": True}
]

# ========== TCP 服务器 ==========
def tcp_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(('0.0.0.0', 1025))
    server.listen(8)
    print("[TCP] Listening on port 1025 ...")

    while True:
        conn, addr = server.accept()
        print(f"[TCP] New connection from {addr}")
        threading.Thread(target=handle_device, args=(conn,), daemon=True).start()

def handle_device(conn: socket.socket):
    buffer = b""
    while True:
        try:
            data = conn.recv(4096)
            if not data:
                break
            buffer += data
            while b"\r\n\r\n" in buffer:
                request_line, _, body = buffer.partition(b"\r\n\r\n")
                buffer = body
                try:
                    headers = request_line.decode('utf-8', errors='ignore')
                except:
                    continue
                if "POST /device/update" in headers:
                    try:
                        json_data = json.loads(body.decode('utf-8'))
                    except:
                        conn.send(b"HTTP/1.1 400 Bad Request\r\n\r\n")
                        continue

                    project_id = json_data.pop("project", 0)
                    project_name = json_data.pop("name", f"Project {project_id}")
                    sensor_info = json_data
                    sensor_info["timestamp"] = time.strftime("%Y-%m-%d %H:%M:%S")

                    with projects_lock:
                        if project_id not in projects:
                            projects[project_id] = {
                                "sensor": {},
                                "conn": None,
                                "name": project_name
                            }
                        projects[project_id]["sensor"].update(sensor_info)
                        projects[project_id]["conn"] = conn
                        projects[project_id]["name"] = project_name

                    print(f"[Project {project_id}] Sensor updated: {sensor_info}")
                    conn.send(b"HTTP/1.1 200 OK\r\n\r\n")
                else:
                    conn.send(b"HTTP/1.1 404 Not Found\r\n\r\n")
        except Exception as e:
            print(f"[TCP] Connection error: {e}")
            break

    with projects_lock:
        for pid, info in list(projects.items()):
            if info["conn"] == conn:
                projects[pid]["conn"] = None
                print(f"[Project {pid}] Device disconnected")
                break
    conn.close()

def send_command_to_device(project_id: int, cmd_char: str):
    with projects_lock:
        if project_id not in projects:
            return False, f"Project {project_id} not registered"
        conn = projects[project_id]["conn"]
        if conn is None:
            return False, "Device not connected"
    try:
        conn.sendall(cmd_char.encode())
        return True, "Command sent"
    except Exception as e:
        with projects_lock:
            projects[project_id]["conn"] = None
        return False, str(e)

# ========== Flask Web 服务 ==========
app = Flask(__name__)

# ---------- 首页：显示所有项目 ----------
@app.route('/')
def index():
    return render_template_string(HOME_HTML, projects=PROJECT_LIST)

# ---------- 项目专属页面 ----------
@app.route('/project/<int:pid>')
def project_page(pid):
    project_info = next((p for p in PROJECT_LIST if p["id"] == pid), None)
    if project_info is None:
        return "Project not found", 404

    # 确保 projects 字典有该项目的条目
    with projects_lock:
        if pid not in projects:
            default_sensor = get_default_sensor(pid)
            projects[pid] = {
                "sensor": default_sensor,
                "conn": None,
                "name": project_info["name"]
            }

    # 模板映射
    templates = {
        0: BASE_HTML_TEMPLATE,
        13: PET_HTML_TEMPLATE,
        16: TRASH_HTML_TEMPLATE,
        15: WINDOW_HTML_TEMPLATE,
        20: CLASSROOM_HTML_TEMPLATE,
        17: FARM_HTML_TEMPLATE,
        25: DRONE_HTML_TEMPLATE,
    }
    template = templates.get(pid, PLACEHOLDER_HTML_TEMPLATE)

    return render_template_string(template, pid=pid, name=project_info["name"])

# ---------- 获取默认传感器数据 ----------
def get_default_sensor(pid: int) -> dict:
    """根据项目ID返回默认传感器字段（所有数值为0）"""
    if pid == 13:   # 宠物屋
        return {
            "temperature": 0.0, "humidity": 0.0, "distance": 0.0,
            "in_nest": 0, "stay_seconds": 0, "feed_reminder": 0,
            "timestamp": ""
        }
    elif pid == 16: # 垃圾桶
        return {
            "distance": 0.0, "capacity": 0, "person_detected": 0,
            "lid_open": 0, "alarm": 0, "compressing": 0,
            "timestamp": ""
        }
    elif pid == 15: # 窗户
        return {
            "temperature": 0.0, "humidity": 0.0, "light": 0.0,
            "pir_detected": 0, "led_brightness": 0, "curtain_pos": 0,
            "usage_rate": 0,"curtain_runtime": 0, "timestamp": ""
        }
    elif pid == 20:#教室
        return {
            "temperature": 0.0, "humidity": 0.0, "light": 0.0,
            "pir_detected": 0,"mode":0.0,"fan_speed": 0,
            "led_brightness": 0, "curtain_pos": 0,"usage": 0,
            "fan_runtime": 0,"light_runtime": 0,"timestamp": ""
        }
    elif pid == 17: # 农场
        return {
            "light": 0.0, "temperature": 0.0, "humidity": 0.0,
            "height": 0.0, "irrigating": 0, "shade_closed": 0,
            "timestamp": ""
        }
    elif pid == 25: # 无人机
        return {
            "altitude": 0.0, "temperature": 0.0, "humidity": 0.0,
            "power_gear": 0, "alarm_flag": 0, "flight_mode": 0,
            "flight_time": 0, "timestamp": ""
        }
    else:           # 基础项目或其他
        return {
            "temperature": 0.0, "humidity": 0.0, "light": 0.0,
            "timestamp": ""
        }

# ---------- API ----------
@app.route('/api/sensor/<int:pid>')
def api_sensor(pid):
    with projects_lock:
        if pid not in projects:
            return jsonify(get_default_sensor(pid))
        data = projects[pid].get("sensor", {})
        online = projects[pid]["conn"] is not None
        data["online"] = online
        return jsonify(data)

@app.route('/api/command/<int:pid>', methods=['POST'])
def api_command(pid):
    data = request.get_json()
    cmd = data.get('cmd', '')
    if not cmd or len(cmd) != 1:
        return jsonify({"status": "error", "message": "Command must be a single character"}), 400
    success, msg = send_command_to_device(pid, cmd)
    return jsonify({"status": "ok" if success else "error", "message": msg})

# ========== HTML 模板 ==========

# 主页模板（保持不变）
HOME_HTML = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>IoT 多项目控制中心</title>
    <style>
        body { font-family: Arial; margin: 40px; text-align: center; }
        .project-grid { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; margin-top: 30px; }
        .project-card {
            border: 1px solid #ccc;
            border-radius: 10px;
            padding: 30px 20px;
            width: 180px;
            text-decoration: none;
            color: #333;
            background: #f9f9f9;
            transition: 0.3s;
        }
        .project-card:hover { background: #e0e0e0; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
        .project-card .name { font-size: 18px; font-weight: bold; }
        .project-card .id { font-size: 14px; color: #888; }
        .project-card .status { font-size: 12px; color: #aaa; margin-top: 8px; }
    </style>
</head>
<body>
    <h1>IoT 多项目控制中心</h1>
    <p>点击项目卡片进入控制页面</p>
    <div class="project-grid">
        {% for p in projects %}
            {% if p.has_page %}
                <a href="/project/{{ p.id }}" class="project-card">
                    <div class="name">{{ p.name }}</div>
                    <div class="id">Project ID: {{ p.id }}</div>
                    <div class="status">✅ 已实现</div>
                </a>
            {% else %}
                <div class="project-card disabled">
                    <div class="name">{{ p.name }}</div>
                    <div class="id">Project ID: {{ p.id }}</div>
                    <div class="status">⏳ 待开发</div>
                </div>
            {% endif %}
        {% endfor %}
    </div>
</body>
</html>
"""

# ===== 基础 IoT 页面（项目0） =====
BASE_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }} - Control</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .data-box { border:1px solid #ccc; padding:20px; margin:20px 0; }
        .btn { margin:5px; padding:10px 20px; font-size:16px; cursor:pointer; }
        #status { margin-top:10px; color:blue; }
    </style>
</head>
<body>
    <h1>{{ name }} (Project {{ pid }})</h1>
    <div class="data-box">
        <h3>Sensor Data</h3>
        <p id="temp">Temperature: 0.0 °C</p>
        <p id="humi">Humidity: 0.0 %</p>
        <p id="light">Illuminance: 0.0 lx</p>
        <p id="time">Last update: --</p>
        <p id="online">Status: Offline</p>
    </div>
    <div>
        <h3>Control Commands</h3>
        <button class="btn" onclick="sendCmd('A')">LED Red ON</button>
        <button class="btn" onclick="sendCmd('E')">LED OFF</button>
        <button class="btn" onclick="sendCmd('B')">Fan 50%</button>
        <button class="btn" onclick="sendCmd('F')">Fan OFF</button>
        <button class="btn" onclick="sendCmd('D')">Curtain 100%</button>
        <button class="btn" onclick="sendCmd('G')">Curtain 0%</button>
        <button class="btn" onclick="sendCmd('C')">Show Temp</button>
        <div style="margin-top:10px;">
            <label>Custom: </label>
            <input type="text" id="custom_cmd" maxlength="1" style="width:30px;">
            <button class="btn" onclick="sendCustom()">Send</button>
        </div>
    </div>
    <div id="status"></div>
    <script>
        const projectId = {{ pid }};
        function fetchData() {
            fetch(`/api/sensor/${projectId}`)
                .then(res => res.json())
                .then(data => {
                    if (data.error) {
                        document.getElementById('online').innerText = 'Status: Offline';
                        return;
                    }
                    document.getElementById('temp').innerText = `Temperature: ${data.temperature ?? 0.0} °C`;
                    document.getElementById('humi').innerText = `Humidity: ${data.humidity ?? 0.0} %`;
                    document.getElementById('light').innerText = `Illuminance: ${data.illuminance ?? 0.0} lx`;
                    document.getElementById('time').innerText = `Last update: ${data.timestamp || '--'}`;
                    document.getElementById('online').innerText = data.online ? 'Status: Online' : 'Status: Offline';
                })
                .catch(() => {
                    document.getElementById('online').innerText = 'Status: Offline';
                });
        }
        function sendCmd(cmd) {
            fetch(`/api/command/${projectId}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: cmd})
            })
            .then(res => res.json())
            .then(data => {
                document.getElementById('status').innerText = data.message;
            });
        }
        function sendCustom() {
            const cmd = document.getElementById('custom_cmd').value.trim();
            if (cmd.length !== 1) {
                alert('Please enter a single character');
                return;
            }
            sendCmd(cmd);
            document.getElementById('custom_cmd').value = '';
        }
        setInterval(fetchData, 2000);
        fetchData();
    </script>
</body>
</html>
"""

# ===== 宠物屋页面（项目13） =====
PET_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }} - Pet House</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .data-box { border:1px solid #ccc; padding:20px; margin:20px 0; }
        .btn { margin:5px; padding:10px 20px; font-size:16px; cursor:pointer; }
        .reminder { color: red; font-weight: bold; }
        #status { margin-top:10px; color:blue; }
    </style>
</head>
<body>
    <h1>{{ name }} (Project {{ pid }})</h1>
    <div class="data-box">
        <h3>Sensor Data</h3>
        <p id="temp">Temperature: 0.0 °C</p>
        <p id="humi">Humidity: 0.0 %</p>
        <p id="dist">Distance: 0.0 cm</p>
        <p id="nest">In Nest: No</p>
        <p id="stay">Stay Duration: 0 s</p>
        <p id="feed">Feed Reminder: <span class="reminder">None</span></p>
        <p id="time">Last update: --</p>
        <p id="online">Status: Offline</p>
    </div>
    <div>
        <h3>Control Commands</h3>
        <button class="btn" onclick="sendCmd('H')">Feed (H)</button>
        <button class="btn" onclick="sendCmd('Q')">Query Status (Q)</button>
        <div style="margin-top:10px;">
            <label>Custom: </label>
            <input type="text" id="custom_cmd" maxlength="1" style="width:30px;">
            <button class="btn" onclick="sendCustom()">Send</button>
        </div>
    </div>
    <div id="status"></div>
    <script>
        const projectId = {{ pid }};
        function fetchData() {
            fetch(`/api/sensor/${projectId}`)
                .then(res => res.json())
                .then(data => {
                    if (data.error) {
                        document.getElementById('online').innerText = 'Status: Offline';
                        return;
                    }
                    document.getElementById('temp').innerText = `Temperature: ${data.temperature ?? 0.0} °C`;
                    document.getElementById('humi').innerText = `Humidity: ${data.humidity ?? 0.0} %`;
                    document.getElementById('dist').innerText = `Distance: ${data.distance ?? 0.0} cm`;
                    document.getElementById('nest').innerText = `In Nest: ${data.in_nest ? 'Yes' : 'No'}`;
                    document.getElementById('stay').innerText = `Stay Duration: ${data.stay_seconds ?? 0} s`;
                    const feed = data.feed_reminder ? '⚠️ Reminder!' : 'None';
                    document.getElementById('feed').innerHTML = `Feed Reminder: <span class="${data.feed_reminder ? 'reminder' : ''}">${feed}</span>`;
                    document.getElementById('time').innerText = `Last update: ${data.timestamp || '--'}`;
                    document.getElementById('online').innerText = data.online ? 'Status: Online' : 'Status: Offline';
                })
                .catch(() => {
                    document.getElementById('online').innerText = 'Status: Offline';
                });
        }
        function sendCmd(cmd) {
            fetch(`/api/command/${projectId}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: cmd})
            })
            .then(res => res.json())
            .then(data => {
                document.getElementById('status').innerText = data.message;
            });
        }
        function sendCustom() {
            const cmd = document.getElementById('custom_cmd').value.trim();
            if (cmd.length !== 1) {
                alert('Please enter a single character');
                return;
            }
            sendCmd(cmd);
            document.getElementById('custom_cmd').value = '';
        }
        setInterval(fetchData, 2000);
        fetchData();
    </script>
</body>
</html>
"""

# ===== 垃圾桶页面（项目16） =====
TRASH_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }} - Smart Trash</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .data-box { border:1px solid #ccc; padding:20px; margin:20px 0; }
        .btn { margin:5px; padding:10px 20px; font-size:16px; cursor:pointer; }
        .alarm { color: red; font-weight: bold; }
        #status { margin-top:10px; color:blue; }
    </style>
</head>
<body>
    <h1>{{ name }} (Project {{ pid }})</h1>
    <div class="data-box">
        <h3>Sensor Data</h3>
        <p id="dist">Distance: 0.0 cm</p>
        <p id="capacity">Capacity: 0%</p>
        <p id="person">Person Detected: No</p>
        <p id="lid">Lid: Closed</p>
        <p id="alarm">Alarm: <span class="alarm">No</span></p>
        <p id="compressing">Compressing: No</p>
        <p id="time">Last update: --</p>
        <p id="online">Status: Offline</p>
    </div>
    <div>
        <h3>Control Commands</h3>
        <button class="btn" onclick="sendCmd('O')">Open Lid (O)</button>
        <button class="btn" onclick="sendCmd('C')">Close Lid (C)</button>
        <button class="btn" onclick="sendCmd('R')">Restore Auto (R)</button>
        <div style="margin-top:10px;">
            <label>Custom: </label>
            <input type="text" id="custom_cmd" maxlength="1" style="width:30px;">
            <button class="btn" onclick="sendCustom()">Send</button>
        </div>
    </div>
    <div id="status"></div>
    <script>
        const projectId = {{ pid }};
        function fetchData() {
            fetch(`/api/sensor/${projectId}`)
                .then(res => res.json())
                .then(data => {
                    if (data.error) {
                        document.getElementById('online').innerText = 'Status: Offline';
                        return;
                    }
                    document.getElementById('dist').innerText = `Distance: ${data.distance ?? 0.0} cm`;
                    document.getElementById('capacity').innerText = `Capacity: ${data.capacity ?? 0}%`;
                    document.getElementById('person').innerText = `Person Detected: ${data.person_detected ? 'Yes' : 'No'}`;
                    document.getElementById('lid').innerText = `Lid: ${data.lid_open ? 'Open' : 'Closed'}`;
                    const alarmSpan = document.getElementById('alarm');
                    alarmSpan.innerHTML = `Alarm: <span class="${data.alarm ? 'alarm' : ''}">${data.alarm ? '⚠️ ALARM' : 'No'}</span>`;
                    document.getElementById('compressing').innerText = `Compressing: ${data.compressing ? 'Yes' : 'No'}`;
                    document.getElementById('time').innerText = `Last update: ${data.timestamp || '--'}`;
                    document.getElementById('online').innerText = data.online ? 'Status: Online' : 'Status: Offline';
                })
                .catch(() => {
                    document.getElementById('online').innerText = 'Status: Offline';
                });
        }
        function sendCmd(cmd) {
            fetch(`/api/command/${projectId}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: cmd})
            })
            .then(res => res.json())
            .then(data => {
                document.getElementById('status').innerText = data.message;
            });
        }
        function sendCustom() {
            const cmd = document.getElementById('custom_cmd').value.trim();
            if (cmd.length !== 1) {
                alert('Please enter a single character');
                return;
            }
            sendCmd(cmd);
            document.getElementById('custom_cmd').value = '';
        }
        setInterval(fetchData, 2000);
        fetchData();
    </script>
</body>
</html>
"""

# ===== 智能窗户页面（项目15） =====
WINDOW_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }} - Smart Window</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .data-box { border:1px solid #ccc; padding:20px; margin:20px 0; }
        .btn { margin:5px; padding:10px 20px; font-size:16px; cursor:pointer; }
        #status { margin-top:10px; color:blue; }
        .field { margin:5px 0; }
    </style>
</head>
<body>
    <h1>{{ name }} (Project {{ pid }})</h1>
    <div class="data-box">
        <h3>Real-time Data</h3>
        <div class="field" id="temp">Temperature: -- °C</div>
        <div class="field" id="humi">Humidity: -- %</div>
        <div class="field" id="light">Light: -- lx</div>
        <div class="field" id="pir">Human presence: --</div>
        <div class="field" id="led">LED brightness: -- %</div>
        <div class="field" id="curtain">Curtain position: -- %</div>
        <div class="field" id="usage">Usage rate (occupied time): -- %</div>
        <div class="field" id="runtime">Curtain runtime: -- s</div>
        <div class="field" id="time">Last update: --</div>
        <div class="field" id="online">Status: Offline</div>
    </div>
    <div>
        <h3>Control Commands</h3>
        <button class="btn" onclick="sendCmd('O')">Open Window (O)</button>
        <button class="btn" onclick="sendCmd('C')">Close Window (C)</button>
        <button class="btn" onclick="sendCmd('A')">Auto Mode (A)</button>
        <div style="margin-top:10px;">
            <label>Custom: </label>
            <input type="text" id="custom_cmd" maxlength="1" style="width:30px;">
            <button class="btn" onclick="sendCustom()">Send</button>
        </div>
    </div>
    <div id="status"></div>
    <script>
        const projectId = {{ pid }};

        function fetchData() {
            fetch(`/api/sensor/${projectId}`)
                .then(res => res.json())
                .then(data => {
                    if (data.error) {
                        document.getElementById('online').innerText = 'Status: Offline';
                        return;
                    }
                    // Update fields with received data
                    document.getElementById('temp').innerText = `Temperature: ${data.temperature ?? 0.0} °C`;
                    document.getElementById('humi').innerText = `Humidity: ${data.humidity ?? 0.0} %`;
                    document.getElementById('light').innerText = `Light: ${data.light ?? 0.0} lx`;
                    document.getElementById('pir').innerText = `Human presence: ${data.pir_detected ? 'Yes' : 'No'}`;
                    document.getElementById('led').innerText = `LED brightness: ${data.led_brightness ?? 0} %`;
                    document.getElementById('curtain').innerText = `Curtain position: ${data.curtain_pos ?? 0} %`;
                    document.getElementById('usage').innerText = `Usage rate (occupied time): ${data.usage_rate ?? 0} %`;
                    document.getElementById('runtime').innerText = `Curtain runtime: ${data.curtain_runtime ?? 0} s`;
                    document.getElementById('time').innerText = `Last update: ${new Date().toLocaleTimeString()}`;
                    document.getElementById('online').innerText = 'Status: Online';
                })
                .catch(() => {
                    document.getElementById('online').innerText = 'Status: Offline';
                });
        }

        function sendCmd(cmd) {
            fetch(`/api/command/${projectId}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: cmd})
            })
            .then(res => res.json())
            .then(data => {
                document.getElementById('status').innerText = data.message || 'Command sent';
            })
            .catch(() => {
                document.getElementById('status').innerText = 'Command failed';
            });
        }

        function sendCustom() {
            const cmd = document.getElementById('custom_cmd').value.trim();
            if (cmd.length !== 1) {
                alert('Please enter a single character');
                return;
            }
            sendCmd(cmd);
            document.getElementById('custom_cmd').value = '';
        }

        // Poll every 2 seconds
        setInterval(fetchData, 2000);
        fetchData();
    </script>
</body>
</html>
"""

# ===== 智能教室页面（项目20） =====
CLASSROOM_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }} - Smart Classroom</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .data-box { border:1px solid #ccc; padding:20px; margin:20px 0; }
        .btn { margin:5px; padding:10px 20px; font-size:16px; cursor:pointer; }
        #status { margin-top:10px; color:blue; }
    </style>
</head>
<body>
    <h1>{{ name }} (Project {{ pid }})</h1>
    <div class="data-box">
        <h3>Sensor Data</h3>
        <p id="temp">Temperature: 0.0 °C</p>
        <p id="humi">Humidity: 0.0 %</p>
        <p id="light">Light: 0.0 lx</p>
        <p id="pir">Person: No</p>
        <p id="mode">Mode: Normal (0)</p>
        <p id="fan">Fan Speed: 0%</p>
        <p id="led">LED Brightness: 0%</p>
        <p id="curtain">Curtain Position: 0%</p>
        <p id="usage">Usage: 0 %</p>                <!-- 原 occupancy 改为 usage -->
        <p id="fan_runtime">Fan Runtime: 0 s</p>
        <p id="light_runtime">Light Runtime: 0 s</p>
        <p id="time">Last update: --</p>
        <p id="online">Status: Offline</p>
    </div>
    <div>
        <h3>Control Commands</h3>
        <button class="btn" onclick="sendCmd('M')">Toggle Mode (M)</button>
        <button class="btn" onclick="sendCmd('F')">Toggle Fan (F)</button>
        <button class="btn" onclick="sendCmd('L')">Toggle Light (L)</button>
        <button class="btn" onclick="sendCmd('O')">OPEN AUTO (O)</button>
        <div style="margin-top:10px;">
            <label>Custom: </label>
            <input type="text" id="custom_cmd" maxlength="1" style="width:30px;">
            <button class="btn" onclick="sendCustom()">Send</button>
        </div>
    </div>
    <div id="status"></div>
    <script>
        const projectId = {{ pid }};
        function fetchData() {
            fetch(`/api/sensor/${projectId}`)
                .then(res => res.json())
                .then(data => {
                    if (data.error) {
                        document.getElementById('online').innerText = 'Status: Offline';
                        return;
                    }
                    document.getElementById('temp').innerText = `Temperature: ${data.temperature ?? 0.0} °C`;
                    document.getElementById('humi').innerText = `Humidity: ${data.humidity ?? 0.0} %`;
                    document.getElementById('light').innerText = `Light: ${data.light ?? 0.0} lx`;
                    document.getElementById('pir').innerText = `Person: ${data.pir_detected ? 'Yes' : 'No'}`;
                    document.getElementById('mode').innerText = `Mode: ${data.mode ?? 0} (${['Normal','Eco','Demo'][data.mode] || 'Unknown'})`;
                    document.getElementById('fan').innerText = `Fan Speed: ${data.fan_speed ?? 0}%`;
                    document.getElementById('led').innerText = `LED Brightness: ${data.led_brightness ?? 0}%`;
                    document.getElementById('curtain').innerText = `Curtain Position: ${data.curtain_pos ?? 0}%`;
                    // 使用 usage 字段替代原来的 occupancy_sec 和 total_sec
                    document.getElementById('usage').innerText = `Usage: ${data.usage ?? 0} %`;
                    document.getElementById('fan_runtime').innerText = `Fan Runtime: ${data.fan_runtime ?? 0} s`;
                    document.getElementById('light_runtime').innerText = `Light Runtime: ${data.light_runtime ?? 0} s`;
                    document.getElementById('time').innerText =  `Last update: ${data.timestamp || '--'}`;
                    // 有数据即表示在线
                    document.getElementById('online').innerText = 'Status: Online';
                })
                .catch(() => {
                    document.getElementById('online').innerText = 'Status: Offline';
                });
        }
        function sendCmd(cmd) {
            fetch(`/api/command/${projectId}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: cmd})
            })
            .then(res => res.json())
            .then(data => {
                document.getElementById('status').innerText = data.message;
            });
        }
        function sendCustom() {
            const cmd = document.getElementById('custom_cmd').value.trim();
            if (cmd.length !== 1) {
                alert('Please enter a single character');
                return;
            }
            sendCmd(cmd);
            document.getElementById('custom_cmd').value = '';
        }
        setInterval(fetchData, 2000);
        fetchData();
    </script>
</body>
</html>
"""
# ===== 智能农场页面（项目17） =====
FARM_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }} - Smart Farm</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .data-box { border:1px solid #ccc; padding:20px; margin:20px 0; }
        .btn { margin:5px; padding:10px 20px; font-size:16px; cursor:pointer; }
        #status { margin-top:10px; color:blue; }
    </style>
</head>
<body>
    <h1>{{ name }} (Project {{ pid }})</h1>
    <div class="data-box">
        <h3>Sensor Data</h3>
        <p id="light">Light: 0.0 lx</p>
        <p id="temp">Temperature: 0.0 °C</p>
        <p id="humi">Humidity: 0.0 %</p>
        <p id="height">Crop Height: 0.0 cm</p>
        <p id="irrigating">Irrigating: No</p>
        <p id="shade">Shade: Open</p>
        <p id="time">Last update: --</p>
        <p id="online">Status: Offline</p>
    </div>
    <div>
        <h3>Control Commands</h3>
        <button class="btn" onclick="sendCmd('I')">Start Irrigation (I)</button>
        <button class="btn" onclick="sendCmd('S')">Stop Irrigation (S)</button>
        <button class="btn" onclick="sendCmd('R')">Restore Auto (R)</button>
        <div style="margin-top:10px;">
            <label>Custom: </label>
            <input type="text" id="custom_cmd" maxlength="1" style="width:30px;">
            <button class="btn" onclick="sendCustom()">Send</button>
        </div>
    </div>
    <div id="status"></div>
    <script>
        const projectId = {{ pid }};
        function fetchData() {
            fetch(`/api/sensor/${projectId}`)
                .then(res => res.json())
                .then(data => {
                    if (data.error) {
                        document.getElementById('online').innerText = 'Status: Offline';
                        return;
                    }
                    document.getElementById('light').innerText = `Light: ${data.light ?? 0.0} lx`;
                    document.getElementById('temp').innerText = `Temperature: ${data.temperature ?? 0.0} °C`;
                    document.getElementById('humi').innerText = `Humidity: ${data.humidity ?? 0.0} %`;
                    document.getElementById('height').innerText = `Crop Height: ${data.height ?? 0.0} cm`;
                    document.getElementById('irrigating').innerText = `Irrigating: ${data.irrigating ? 'Yes' : 'No'}`;
                    document.getElementById('shade').innerText = `Shade: ${data.shade_closed ? 'Closed' : 'Open'}`;
                    document.getElementById('time').innerText = `Last update: ${data.timestamp || '--'}`;
                    document.getElementById('online').innerText = data.online ? 'Status: Online' : 'Status: Offline';
                })
                .catch(() => {
                    document.getElementById('online').innerText = 'Status: Offline';
                });
        }
        function sendCmd(cmd) {
            fetch(`/api/command/${projectId}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: cmd})
            })
            .then(res => res.json())
            .then(data => {
                document.getElementById('status').innerText = data.message;
            });
        }
        function sendCustom() {
            const cmd = document.getElementById('custom_cmd').value.trim();
            if (cmd.length !== 1) {
                alert('Please enter a single character');
                return;
            }
            sendCmd(cmd);
            document.getElementById('custom_cmd').value = '';
        }
        setInterval(fetchData, 2000);
        fetchData();
    </script>
</body>
</html>
"""

# ===== 智能无人机页面（项目25） =====
DRONE_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }} - Smart Drone</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .data-box { border:1px solid #ccc; padding:20px; margin:20px 0; }
        .btn { margin:5px; padding:10px 20px; font-size:16px; cursor:pointer; }
        .alarm { color: red; font-weight: bold; }
        #status { margin-top:10px; color:blue; }
    </style>
</head>
<body>
    <h1>{{ name }} (Project {{ pid }})</h1>
    <div class="data-box">
        <h3>Sensor Data</h3>
        <p id="altitude">Altitude: 0.0 cm</p>
        <p id="temp">Temperature: 0.0 °C</p>
        <p id="humi">Humidity: 0.0 %</p>
        <p id="gear">Power Gear: 0</p>
        <p id="alarm_flag">Alarm: <span class="alarm">No</span></p>
        <p id="flight_mode">Flight Mode: Standby</p>
        <p id="flight_time">Flight Time: 0 s</p>
        <p id="time">Last update: --</p>
        <p id="online">Status: Offline</p>
    </div>
    <div>
        <h3>Control Commands</h3>
        <button class="btn" onclick="sendCmd('T')">Takeoff (T)</button>
        <button class="btn" onclick="sendCmd('L')">Land (L)</button>
        <button class="btn" onclick="sendCmd('H')">Hover (H)</button>
        <button class="btn" onclick="sendCmd('U')">Up (U)</button>
        <button class="btn" onclick="sendCmd('D')">Down (D)</button>
        <div style="margin-top:10px;">
            <label>Custom: </label>
            <input type="text" id="custom_cmd" maxlength="1" style="width:30px;">
            <button class="btn" onclick="sendCustom()">Send</button>
        </div>
    </div>
    <div id="status"></div>
    <script>
        const projectId = {{ pid }};
        function fetchData() {
            fetch(`/api/sensor/${projectId}`)
                .then(res => res.json())
                .then(data => {
                    if (data.error) {
                        document.getElementById('online').innerText = 'Status: Offline';
                        return;
                    }
                    document.getElementById('altitude').innerText = `Altitude: ${data.altitude ?? 0.0} cm`;
                    document.getElementById('temp').innerText = `Temperature: ${data.temperature ?? 0.0} °C`;
                    document.getElementById('humi').innerText = `Humidity: ${data.humidity ?? 0.0} %`;
                    document.getElementById('gear').innerText = `Power Gear: ${data.power_gear ?? 0}`;
                    const alarmSpan = document.getElementById('alarm_flag');
                    alarmSpan.innerHTML = `Alarm: <span class="${data.alarm_flag ? 'alarm' : ''}">${data.alarm_flag ? '⚠️ ABNORMAL' : 'No'}</span>`;
                    const modeMap = {0:'Standby',1:'Takeoff',2:'Hover',3:'Land'};
                    document.getElementById('flight_mode').innerText = `Flight Mode: ${modeMap[data.flight_mode] || 'Unknown'}`;
                    document.getElementById('flight_time').innerText = `Flight Time: ${data.flight_time ?? 0} s`;
                    document.getElementById('time').innerText = `Last update: ${data.timestamp || '--'}`;
                    document.getElementById('online').innerText = data.online ? 'Status: Online' : 'Status: Offline';
                })
                .catch(() => {
                    document.getElementById('online').innerText = 'Status: Offline';
                });
        }
        function sendCmd(cmd) {
            fetch(`/api/command/${projectId}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: cmd})
            })
            .then(res => res.json())
            .then(data => {
                document.getElementById('status').innerText = data.message;
            });
        }
        function sendCustom() {
            const cmd = document.getElementById('custom_cmd').value.trim();
            if (cmd.length !== 1) {
                alert('Please enter a single character');
                return;
            }
            sendCmd(cmd);
            document.getElementById('custom_cmd').value = '';
        }
        setInterval(fetchData, 2000);
        fetchData();
    </script>
</body>
</html>
"""

# ===== 占位页面（未实现的项目） =====
PLACEHOLDER_HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{{ name }}</title>
    <style>
        body { font-family: Arial; margin: 40px; text-align: center; }
        .placeholder { margin-top: 50px; color: #888; }
    </style>
</head>
<body>
    <h1>{{ name }}</h1>
    <div class="placeholder">
        <h2>⏳ 项目正在开发中，敬请期待...</h2>
        <p>Project ID: {{ pid }}</p>
        <p><a href="/">返回首页</a></p>
    </div>
</body>
</html>
"""

# ========== 启动服务 ==========
if __name__ == "__main__":
    tcp_thread = threading.Thread(target=tcp_server, daemon=True)
    tcp_thread.start()
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)