param(
    [string]$ServerIp    = "127.0.0.1",
    [int]   $ServerPort  = 8081,
    [int]   $ProjectId   = 20,
    [string]$ProjectName = "Classroom",
    [int]   $Count       = 10,
    [int]   $IntervalSec = 3,
    [switch]$Continuous
)

Write-Output "=========================================="
Write-Output " Mock Sensor Data -> TCP $ServerIp`:$ServerPort"
Write-Output " Project : #$ProjectId ($ProjectName)"
Write-Output "=========================================="

# 用 Python 来发（更稳定，避免 PowerShell 类型坑）
$pyCode = @"
import socket, json, random, time, sys

server = sys.argv[1]
port = int(sys.argv[2])
pid = int(sys.argv[3])
pname = sys.argv[4]
count = int(sys.argv[5])
interval = int(sys.argv[6])
continuous = sys.argv[7] == "1"

seq = 0
while True:
    seq += 1
    temp = round(20.0 + random.random() * 10.0, 1)
    humi = round(40.0 + random.random() * 50.0, 1)
    light = random.randint(50, 1000)
    pir = random.randint(0, 1)

    payload = {
        "project": pid,
        "name": pname,
        "temperature": temp,
        "humidity": humi,
        "light": light,
        "pir_detected": pir
    }
    json_str = json.dumps(payload, ensure_ascii=False)

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(3)
        s.connect((server, port))
        s.sendall((json_str + "\r\n").encode("utf-8"))
        time.sleep(0.3)
        resp = ""
        try:
            resp = s.recv(4096).decode("utf-8", errors="replace")
        except:
            pass
        s.close()

        status = "OK" if "200 OK" in resp or '"status":"OK"' in resp else "FAIL"
        print(f"[{time.strftime('%H:%M:%S')}] {status} #{seq}  temp={temp} humi={humi} light={light} pir={pir}")
        sys.stdout.flush()
    except Exception as e:
        print(f"[ERR] #{seq}  {e}")
        sys.stdout.flush()

    if not continuous and seq >= count:
        print(f"\nDone. Sent {seq} records.")
        break

    time.sleep(interval)
"@

$pyArgs = @(
    "-c", $pyCode,
    $ServerIp, $ServerPort, $ProjectId, $ProjectName,
    $(if ($Continuous) { 0 } else { $Count }),
    $IntervalSec,
    $(if ($Continuous) { 1 } else { 0 })
)

python @pyArgs
