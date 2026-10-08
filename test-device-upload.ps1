# 模拟设备向 IoT 服务端上报一次数据，用于自检服务器端是否正常
$body = '{"project":20,"name":"智能教室","temperature":26.5,"humidity":61,"light":320}'
$bytes = [System.Text.Encoding]::UTF8.GetBytes($body)
$req = "POST /device/update HTTP/1.1`r`nHost: 127.0.0.1:1025`r`nContent-Type: application/json`r`nContent-Length: $($bytes.Length)`r`nConnection: close`r`n`r`n"
$reqBytes = [System.Text.Encoding]::UTF8.GetBytes($req)

try {
    $client = New-Object System.Net.Sockets.TcpClient('127.0.0.1', 1025)
    $stream = $client.GetStream()
    $stream.Write($reqBytes, 0, $reqBytes.Length)
    $stream.Write($bytes, 0, $bytes.Length)
    $stream.Flush()
    Start-Sleep -Milliseconds 800
    $buf = New-Object byte[] 2048
    $n = $stream.Read($buf, 0, 2048)
    if ($n -gt 0) {
        $resp = [System.Text.Encoding]::UTF8.GetString($buf, 0, $n)
        Write-Host "服务器响应:"
        Write-Host $resp
    } else {
        Write-Host "服务器未返回数据"
    }
    $client.Close()
} catch {
    Write-Host "连接失败: $($_.Exception.Message)"
}
