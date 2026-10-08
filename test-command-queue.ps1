# 完整模拟短连接设备场景：
# 1) 上报一次（发完即断）
# 2) 此时 online=true 但 tcpConnected=false
# 3) Web 下发命令 -> 应返回 queued（排队，不再报错）
# 4) 设备再次上报 -> HTTP 响应里应带回 {"cmd":"M"}
param(
  [string]$CmdToQueue = "M"
)

function Send-Upload {
  $body = '{"project":20,"temperature":26.0,"humidity":55,"light":300}'
  $bytes = [System.Text.Encoding]::UTF8.GetBytes($body)
  $client = New-Object System.Net.Sockets.TcpClient('127.0.0.1', 1025)
  $stream = $client.GetStream()
  $stream.Write($bytes, 0, $bytes.Length)
  $stream.Flush()
  Start-Sleep -Milliseconds 600
  $buf = New-Object byte[] 2048
  $n = $stream.Read($buf, 0, 2048)
  $resp = [System.Text.Encoding]::UTF8.GetString($buf, 0, $n)
  $client.Close()
  return $resp
}

Start-Sleep -Seconds 6

Write-Host "=== 步骤1: 设备第一次上报（短连接） ==="
Send-Upload | Out-Null
Write-Host "已上报并断开"

Write-Host "`n=== 步骤2: 查询状态 online / tcpConnected ==="
$s = (Invoke-WebRequest -Uri 'http://127.0.0.1:8080/api/sensor/20' -UseBasicParsing).Content
Write-Host $s

Write-Host "`n=== 步骤3: Web 下发命令 $CmdToQueue（设备 TCP 没连着，应排队不报错） ==="
$r = Invoke-WebRequest -Uri 'http://127.0.0.1:8080/api/command/20' -Method POST -ContentType 'application/json' -Body ('{"cmd":"' + $CmdToQueue + '"}') -UseBasicParsing
Write-Host "HTTP $($r.StatusCode): $($r.Content)"

Write-Host "`n=== 步骤4: 设备第二次上报，响应中应携带 cmd ==="
$resp = Send-Upload
Write-Host "设备收到的 HTTP 响应:"
Write-Host $resp
