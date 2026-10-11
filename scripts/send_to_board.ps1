param(
    [string]$BackendUrl  = "http://localhost:8080",
    [int]   $ProjectId   = 0,
    [string]$Device      = "gd32",
    [string]$Message     = "5678",
    [int]   $IntervalSec = 3,
    [int]   $MaxSend     = 1,
    [int]   $WaitTimeoutSec = 30
)

$ErrorActionPreference = "Continue"
$seq = 0

Write-Output "=========================================="
Write-Output " Send To IoT Board"
Write-Output " Backend : $BackendUrl"
Write-Output " Message : $Message"
Write-Output "=========================================="

# 1. wait backend
$backendReady = $false
for ($i = 0; $i -lt 20; $i++) {
    try {
        $null = Invoke-RestMethod "$BackendUrl/api/system/health" -TimeoutSec 2
        $backendReady = $true
        break
    } catch {
        Start-Sleep -Seconds 1
    }
}
if (-not $backendReady) {
    Write-Output "Backend not ready. Start it first."
    exit 1
}
Write-Output "Backend OK"

# 2. wait board online
Write-Output "Waiting for board..."
for ($i = 0; $i -lt ($WaitTimeoutSec / 2); $i++) {
    try {
        $h = Invoke-RestMethod "$BackendUrl/api/system/health" -TimeoutSec 2
        if ($h.onlineDevices -gt 0) {
            Write-Output "Board online"
            break
        }
    } catch {}
    Start-Sleep -Seconds 2
}

# 3. send loop
Write-Output "Start sending..."

while ($true) {
    try {
        $seq++
        $boardJson = '{"device":"' + $Device + '","message":"' + $Message + '","seq":' + $seq + '}'

        $bodyObj = @{ projectId = $ProjectId; data = $boardJson }
        $bodyStr = $bodyObj | ConvertTo-Json -Compress

        $resp = Invoke-RestMethod -Uri "$BackendUrl/api/tcp-send" -Method POST -ContentType "application/json" -Body $bodyStr -TimeoutSec 5

        if ($resp.status -eq "ok") {
            Write-Output ("[{0}] OK #{1} -> {2}" -f (Get-Date -Format HH:mm:ss), $seq, $boardJson)
        } else {
            Write-Output ("[{0}] ERR #{1} : {2}" -f (Get-Date -Format HH:mm:ss), $seq, $resp.message)
        }
    } catch {
        Write-Output ("[{0}] FAIL #{1} : {2}" -f (Get-Date -Format HH:mm:ss), $seq, $_.Exception.Message)
    }

    if ($MaxSend -gt 0 -and $seq -ge $MaxSend) {
        Write-Output "Done. Total: $seq"
        break
    }

    Start-Sleep -Seconds $IntervalSec
}
