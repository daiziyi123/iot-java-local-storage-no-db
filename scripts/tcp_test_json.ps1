# TCP JSON Auto Sender
# 每隔 N 秒向 TCP 服务器发送符合后端协议的 HTTP POST JSON 帧
#
# 运行：
#   .\scripts\tcp_test_json.ps1                 # 默认 Classroom，无限循环
#   .\scripts\tcp_test_json.ps1 -MaxSend 10     # 发 10 次停
#   .\scripts\tcp_test_json.ps1 -ProjectId 15 -ProjectName "Window"

param(
    [string]$ServerIp   = "127.0.0.1",
    [int]   $ServerPort = 8081,
    [int]   $IntervalSec = 3,
    [int]   $MaxSend     = 0,
    [int]   $ProjectId   = 20,
    [string]$ProjectName = "Classroom"
)

$ErrorActionPreference = "Continue"
$client = $null
$sendCount = 0

function New-JsonBody($prjId, $pname) {
    $t  = [Math]::Round(23.0 + (Get-Random -Minimum 0 -Maximum 70) / 10.0, 1)
    $h  = [Math]::Round(45.0 + (Get-Random -Minimum 0 -Maximum 40) / 10.0, 1)
    $li = Get-Random -Minimum 300 -Maximum 800
    $pd  = Get-Random -Minimum 0 -Maximum 2
    return '{"project":' + $prjId + ',' +
           '"name":"' + $pname + '",' +
           '"temperature":' + $t + ',' +
           '"humidity":' + $h + ',' +
           '"light":' + $li + ',' +
           '"pir_detected":' + $pd + '}'
}

function New-HttpFrame($body, $hst, $port) {
    $bl = $body.Length
    return 'POST /device/update HTTP/1.1' + "`r`n" +
           'Host: ' + $hst + ':' + $port + "`r`n" +
           'Content-Type: application/json' + "`r`n" +
           'Content-Length: ' + $bl + "`r`n" +
           "`r`n" + $body + "`r`n`r`n"
}

Write-Output "=========================================="
Write-Output " TCP JSON Auto Sender"
Write-Output " Target   : ${ServerIp}:${ServerPort}"
Write-Output " Project  : #${ProjectId} ${ProjectName}"
Write-Output " Interval : ${IntervalSec}s"
Write-Output " Stop     : $(if ($MaxSend -gt 0) {"$MaxSend sends"} else {"Ctrl+C"})"
Write-Output "=========================================="

while ($true) {
    try {
        if (-not $client -or -not $client.Connected) {
            if ($client) { $client.Close() }
            Write-Output ("[{0}] Connecting {1}:{2} ..." -f (Get-Date -Format HH:mm:ss), $ServerIp, $ServerPort)
            $client = New-Object System.Net.Sockets.TcpClient
            $client.Connect($ServerIp, $ServerPort)
            Write-Output ("[{0}] Connected" -f (Get-Date -Format HH:mm:ss))
        }

        $json  = New-JsonBody $ProjectId $ProjectName
        $frame = New-HttpFrame $json $ServerIp $ServerPort
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($frame)

        $stream = $client.GetStream()
        $stream.Write($bytes, 0, $bytes.Length)
        $sendCount++

        Write-Output ("[{0}] -> #{1} ({2} bytes)" -f (Get-Date -Format HH:mm:ss), $sendCount, $bytes.Length)
        Write-Output ("    JSON: {0}" -f $json)

        # 读响应（3s 超时）
        $stream.ReadTimeout = 3000
        $reader = New-Object System.IO.StreamReader($stream)
        $resp = ""
        try {
            while (-not $reader.EndOfStream) {
                $line = $reader.ReadLine()
                if ($null -eq $line) { break }
                $resp += $line + "`n"
                if ($line.StartsWith("{")) { break }
            }
        } catch { }

        if ($resp) {
            Write-Output ("[{0}] <- {1}" -f (Get-Date -Format HH:mm:ss), $resp.Trim())
        } else {
            Write-Output ("[{0}] <- (no response)" -f (Get-Date -Format HH:mm:ss))
        }

        if ($MaxSend -gt 0 -and $sendCount -ge $MaxSend) {
            Write-Output "Done. Sent $MaxSend frames."
            break
        }

    } catch {
        Write-Output ("[{0}] Error: {1}" -f (Get-Date -Format HH:mm:ss), $_.Exception.Message)
        try { if ($client) { $client.Close() } } catch {}
        $client = $null
    }

    Start-Sleep -Seconds $IntervalSec
}

if ($client) { $client.Close() }
Write-Output "End. Total: $sendCount frames."
