#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
交互式 Mock 传感器数据发送器（多项目并发版）
=============================================
直接运行进入交互菜单: python mock_sensor_data.py
命令行模式兼容旧版:   python mock_sensor_data.py --project-id 20 --count 10

交互模式：
  输入项目ID (20)           → 启动该项目后台线程，循环发数据
  输入多个项目ID (20,15,13) → 同时启动多个项目
  list / l                  → 查看当前运行的项目
  stop 20                   → 停止项目20
  stop all                  → 停止所有
  s                         → 修改后端地址
  q / quit                  → 退出程序（自动停所有线程）
"""

import socket
import json
import random
import time
import sys
import os
import threading


# ============================================================
# 项目 ID → 名称映射（与后端 TcpDeviceServer.defaultProjectName 一致）
# ============================================================
PROJECT_MAP = {
    0:  "基础 IoT 控制",
    13: "智能宠物屋",
    15: "智能窗户",
    16: "智能垃圾桶",
    17: "智能农场",
    20: "智能教室",
    25: "智能无人机",
}


# ============================================================
# 场景生成器（只保留"正常随机"，输入项目ID即自动使用）
# ============================================================
def gen_data(pid, pname):
    """生成一条随机传感器数据"""
    temp = round(20.0 + random.random() * 10.0, 1)
    humi = round(40.0 + random.random() * 50.0, 1)
    hour = time.localtime().tm_hour
    light = random.randint(300, 1000) if 8 <= hour < 18 else random.randint(0, 100)
    pir = 1 if random.random() < 0.4 else 0
    return {"project": pid, "name": pname,
            "temperature": temp, "humidity": humi,
            "light": light, "pir_detected": pir}


# ============================================================
# TCP 发送
# ============================================================
def send_once(server, port, payload):
    json_str = json.dumps(payload, ensure_ascii=False)
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect((server, port))
        sock.sendall((json_str + "\r\n").encode("utf-8"))
        time.sleep(0.3)
        try:
            sock.recv(4096)
        except socket.timeout:
            pass
        sock.close()
        return True
    except Exception:
        return False


# ============================================================
# 后台发送线程（每个项目一个线程）
# ============================================================
class MockProjectThread(threading.Thread):
    def __init__(self, project_id, pname, server, port, interval=3):
        super().__init__(daemon=True, name=f"Mock-{project_id}")
        self.project_id = project_id
        self.pname = pname
        self.server = server
        self.port = port
        self.interval = interval
        self._stop_event = threading.Event()
        self.seq = 0
        self.last_ok = 0    # 上次成功时间戳
        self.last_fail = 0  # 上次失败时间戳

    def run(self):
        while not self._stop_event.is_set():
            self.seq += 1
            payload = gen_data(self.project_id, self.pname)
            ok = send_once(self.server, self.port, payload)
            now = time.strftime("%H:%M:%S")
            pir_str = "有人" if payload["pir_detected"] else "无人"
            if ok:
                self.last_ok = time.time()
                print(f"\033[92m[{now}] #{self.project_id} OK #{self.seq:>3}\033[0m"
                      f"  t={payload['temperature']:>5}C h={payload['humidity']:>5}%"
                      f"  l={payload['light']:>4} pir={pir_str}", flush=True)
            else:
                self.last_fail = time.time()
                print(f"\033[91m[{now}] #{self.project_id} FAIL #{self.seq:>3}\033[0m", flush=True)
            # 等 interval 秒，或被 stop 立即打断
            self._stop_event.wait(self.interval)

    def stop(self):
        self._stop_event.set()

    def is_running(self):
        return not self._stop_event.is_set()


# ============================================================
# 交互菜单
# ============================================================
def clear_screen():
    os.system("cls" if os.name == "nt" else "clear")


def project_name_of(pid):
    return PROJECT_MAP.get(pid, f"Project {pid}")


def show_header(server, port):
    print("=" * 60)
    print("  Mock 多项目模拟器（输入项目ID即启动）")
    print("=" * 60)
    print(f"  后端     : {server}:{port}")
    print(f"  时间     : {time.strftime('%H:%M:%S')}")
    print("-" * 60)
    print("  输入项目ID启动（用逗号分隔可同时启动多个）:")
    items = sorted(PROJECT_MAP.items())
    for i in range(0, len(items), 2):
        left = f"{items[i][0]:>3} = {items[i][1]}"
        right = ""
        if i + 1 < len(items):
            right = f"{items[i+1][0]:>3} = {items[i+1][1]}"
        print(f"    {left:<28s} {right}")
    print("-" * 60)
    print("  list / l              查看运行中项目")
    print("  stop <pid|all>        停止项目（例: stop 20, stop all）")
    print("  s                     修改后端地址")
    print("  q / quit              退出程序")
    print("=" * 60)


def show_running(threads):
    if not threads:
        print("  （暂无运行中的项目）")
        return
    print(f"  运行中 {len(threads)} 个项目:")
    for pid, t in sorted(threads.items()):
        status = "\033[92m✓\033[0m" if t.is_alive() else "\033[91m✗\033[0m"
        print(f"    {status} #{pid} {t.pname:<10s} seq={t.seq}  interval={t.interval}s")


def run_interactive():
    server, port = "127.0.0.1", 8081
    # project_id → MockProjectThread
    threads = {}

    def start_project(pid):
        if pid in threads and threads[pid].is_alive():
            print(f"  #{pid} 已在运行中")
            return
        pname = project_name_of(pid)
        t = MockProjectThread(pid, pname, server, port, interval=3)
        threads[pid] = t
        t.start()
        print(f"  \033[92m✓ 启动 #{pid} ({pname})\033[0m  每 3 秒发一次")

    def stop_project(pid):
        if pid in threads:
            threads[pid].stop()
            threads[pid].join(timeout=1)
            del threads[pid]
            print(f"  \033[93m■ 停止 #{pid}\033[0m")
        else:
            print(f"  #{pid} 没在运行")

    def stop_all():
        for pid in list(threads.keys()):
            stop_project(pid)

    while True:
        try:
            ui = input("\n> ").strip().lower()
        except (EOFError, KeyboardInterrupt):
            print("\n退出中..."); stop_all(); print("再见！"); break

        if not ui:
            continue

        if ui in ("q", "quit", "exit"):
            stop_all(); print("再见！"); break

        if ui in ("l", "list"):
            show_running(threads); continue

        if ui.startswith("stop"):
            parts = ui.split()
            if len(parts) < 2:
                print("用法: stop <项目ID|all>"); continue
            target = parts[1]
            if target == "all":
                stop_all()
            else:
                try:
                    stop_project(int(target))
                except ValueError:
                    print(f"无效项目ID: {target}")
            continue

        if ui == "s":
            print(f"  当前后端: {server}:{port}")
            s = input("  新地址 (回车保持): ").strip()
            if s:
                server = s
                try:
                    p = input("  新端口 (回车保持): ").strip()
                    if p: port = int(p)
                except ValueError:
                    pass
                # 重启所有已有线程使新地址生效
                if threads:
                    print("  后端地址变更，重启所有项目...")
                    pids = list(threads.keys())
                    stop_all()
                    for pid in pids: start_project(pid)
            continue

        # 核心：尝试把输入按逗号分割，每个数字当成项目 ID
        # 也支持空格、分号、中文逗号分隔
        for sep in [",", " ", ";", "，"]:
            ui = ui.replace(sep, "|")
        parts = [p for p in ui.split("|") if p]

        launched = 0
        for p in parts:
            p = p.strip()
            if not p:
                continue
            try:
                pid = int(p)
            except ValueError:
                print(f"  跳过非数字: {p}"); continue
            start_project(pid); launched += 1

        if launched == 0 and all(not p.isdigit() for p in parts):
            # 可能用户没看到菜单，显示一下
            print("\n  提示: 输入项目ID启动，例: 20  或  20,15,13  同时启动多个")
            print("        list 看运行中  stop 20 停  stop all 全停  q 退出")
            time.sleep(1)


# ============================================================
# CLI 模式（兼容旧版）
# ============================================================
def run_cli():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--server", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8081)
    parser.add_argument("--project-id", type=int, default=20)
    parser.add_argument("--interval", type=int, default=3)
    parser.add_argument("--continuous", action="store_true")
    parser.add_argument("--count", type=int, default=0,
                        help="发送 N 条后退出（0=无限，需配合 --continuous）")
    args = parser.parse_args()

    pname = project_name_of(args.project_id)
    print("=" * 50)
    print(f" Mock -> {args.server}:{args.port}")
    print(f" Project : #{args.project_id} ({pname})")
    print(f" Mode    : {'Continuous' if args.continuous or args.count == 0 else str(args.count) + ' records'}, {args.interval}s")
    print("=" * 50, flush=True)

    seq = 0
    target = args.count if args.count > 0 else 0xFFFFFFFF
    try:
        while seq < target:
            seq += 1
            payload = gen_data(args.project_id, pname)
            ok = send_once(args.server, args.port, payload)
            ts = time.strftime("%H:%M:%S")
            pir_str = "有人" if payload["pir_detected"] else "无人"
            if ok:
                print(f"\033[92m[{ts}] OK #{seq:>3}\033[0m"
                      f"  t={payload['temperature']:>5}C h={payload['humidity']:>5}%"
                      f"  l={payload['light']:>4} pir={pir_str}", flush=True)
            else:
                print(f"\033[91m[{ts}] FAIL #{seq:>3}\033[0m", flush=True)
            time.sleep(args.interval)
    except KeyboardInterrupt:
        print(f"\nStopped. {seq} records total.")


def main():
    run_cli() if len(sys.argv) > 1 else run_interactive()


if __name__ == "__main__":
    main()
