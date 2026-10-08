# 测试设备端只发裸 JSON（无 HTTP 头）能不能被兼容处理
$body = '{"project":20,"temperature":27.5,"humidity":62,"light":350}'
$bytes = [System.Text.Encoding]::UTF8.GetBytes($body)

try {
    $client = New-Object System.Net.Sockets.TcpClient('127.0.0.1', 1025)
    $stream = $client.GetStream()
    $stream.Write($bytes, 0, $bytes.Length)
    $stream.Flush()
    Start-Sleep -Milliseconds 800
    $buf = New-Object byte[] 2048
    $n = $stream.Read($buf, 0, 2048)
    if ($n -gt 0) {
        $resp = [System.Text.Encoding]::UTF8.GetString($buf, 0, $n)
        Write-Host "Server response:"
        Write-Host $resp
    } else {
        Write-Host "No response from server"
    }
    $client.Close()
} catch {
    Write-Host "Connection failed: $($_.Exception.Message)"
}
