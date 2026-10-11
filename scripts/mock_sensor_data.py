#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
模拟 GD32/ESP32-C3 通过 TCP 8081 向 Java 后端上报传感器数据
后端会把这些数据当成板子真实上报，存入 data/ 文件

用法:
  python mock_sensor_data.py                        # 发10条随机数据
  python mock_sensor_data.py --count 5 --interval 2 # 发5条，间隔2秒
  python mock_sensor_data.py --continuous --interval 3  # 无限循环(Ctrl+C停)
  python mock_sensor_data.py --project-id 15 --project-name Window  # 模拟不同项目
  python mock_sensor_data.py --server 192.168.1.100 --port 8081  # 指定后端地址
"""

import socket
import json
import random
import time
import argparse
import sys


def send_once(server, port, project_id, project_name):
    """发一条 mock 数据，返回 (success, json_str, resp)"""
    temp = round(20.0 + random.random() * 10.0, 1)   # 20.0 ~ 30.0
    humi = round(40.0 + random.random() * 50.0, 1)   # 40.0 ~ 90.0
    light = random.randint(50, 1000)                  # 50 ~ 1000 lux
    pir = random.randint(0, 1)                        # 0 或 1

    payload = {
        "project": project_id,
        "name": project_name,
        "temperature": temp,
        "humidity": humi,
        "light": light,
        "pir_detected": pir
    }
    json_str = json.dumps(payload, ensure_ascii=False)

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect((server, port))
        sock.sendall((json_str + "\r\n").encode("utf-8"))
        time.sleep(0.3)
        resp = ""
        try:
            resp = sock.recv(4096).decode("utf-8", errors="replace")
        except socket.timeout:
            pass
        sock.close()
        ok = "200 OK" in resp or '"status":"OK"' in resp
        return ok, json_str, resp, temp, humi, light, pir
    except Exception as e:
        return False, json_str, str(e), temp, humi, light, pir


def main():
    parser = argparse.ArgumentParser(description="Mock IoT sensor data sender")
    parser.add_argument("--server", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8081)
    parser.add_argument("--project-id", type=int, default=20)
    parser.add_argument("--project-name", default="Classroom")
    parser.add_argument("--count", type=int, default=10, help="发送次数（默认10）")
    parser.add_argument("--interval", type=int, default=3, help="间隔秒数")
    parser.add_argument("--continuous", action="store_true", help="无限循环")
    args = parser.parse_args()

    print("=" * 50)
    print(f" Mock Sensor Data -> TCP {args.server}:{args.port}")
    print(f" Project : #{args.project_id} ({args.project_name})")
    mode = "Continuous (Ctrl+C)" if args.continuous else f"{args.count} records, {args.interval}s interval"
    print(f" Mode    : {mode}")
    print("=" * 50)
    sys.stdout.flush()

    seq = 0
    try:
        while True:
            seq += 1
            ok, json_str, resp, temp, humi, light, pir = send_once(
                args.server, args.port, args.project_id, args.project_name
            )
            ts = time.strftime("%H:%M:%S")
            if ok:
                print(f"[{ts}] OK #{seq}  temp={temp} humi={humi} light={light} pir={pir}")
            else:
                print(f"[{ts}] FAIL #{seq}  {json_str}")
                if resp and resp not in ("", "Connection refused"):
                    print(f"       err: {resp}")
            sys.stdout.flush()

            if not args.continuous and seq >= args.count:
                print(f"\nDone. Sent {seq} records.")
                break

            time.sleep(args.interval)
    except KeyboardInterrupt:
        print(f"\nStopped. Sent {seq} records total.")


if __name__ == "__main__":
    main()
