# TCP 客户端自动化脚本
# 每隔 3 秒向 TCP 服务器 127.0.0.1:8081 发送字符串 "5678"
# 用途：测试后端 TcpDeviceServer 能否正常接收 TCP 数据
#
# 运行：powershell -ExecutionPolicy Bypass -File .\scripts\tcp_test_client.ps1
# 自定义：.\scripts\tcp_test_client.ps1 -IntervalSec 5 -Payload "hello" -MaxSend 10

param(
    [string]$ServerIp = "127.0.0.1",
    [int]$ServerPort  = 8081,
    [int]$IntervalSec = 3,
    [string]$Payload  = "5678",
    [int]$MaxSend     = 0     # 0 = infinite loop, Ctrl+C to stop
)

$ErrorActionPreference = "Continue"
$client = $null
$sendCount = 0

Write-Output "=========================================="
Write-Output " TCP Test Client (Auto Sender)"
Write-Output " Target  : ${ServerIp}:${ServerPort}"
Write-Output " Interval: ${IntervalSec}s"
Write-Output " Payload : $Payload"
if ($MaxSend -gt 0) { Write-Output " Stop    : after $MaxSend sends" } else { Write-Output " Stop    : Ctrl+C" }
Write-Output "=========================================="

while ($true) {
    try {
        if (-not $client -or -not $client.Connected) {
            if ($client) { $client.Close() }
            Write-Output "[$(Get-Date -Format HH:mm:ss)] Connecting ${ServerIp}:${ServerPort} ..."
            $client = New-Object System.Net.Sockets.TcpClient
            $client.Connect($ServerIp, $ServerPort)
            Write-Output "[$(Get-Date -Format HH:mm:ss)] Connected"
        }

        # send payload + CRLF
        $stream = $client.GetStream()
        $bytes  = [System.Text.Encoding]::UTF8.GetBytes($Payload + "`r`n")
        $stream.Write($bytes, 0, $bytes.Length)
        $sendCount++
        Write-Output ("[{0}] -> Sent ({1} bytes): {2}  (count {3})" -f (Get-Date -Format HH:mm:ss), $bytes.Length, $Payload, $sendCount)

        # try read response (1s timeout)
        $stream.ReadTimeout = 1000
        $reader = New-Object System.IO.StreamReader($stream)
        try {
            $resp = $reader.ReadLine()
            if ($resp) {
                Write-Output ("[{0}] <- Response: {1}" -f (Get-Date -Format HH:mm:ss), $resp)
            }
        } catch {
            # ReadTimeout OK
        }

        if ($MaxSend -gt 0 -and $sendCount -ge $MaxSend) {
            Write-Output "Done. Sent $MaxSend times."
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
Write-Output "Script ended. Total sent: $sendCount"
