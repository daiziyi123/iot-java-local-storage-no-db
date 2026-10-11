@echo off
REM ============================================================
REM  一键启动 IoT 后端（CMD 版）
REM  端口: 8080 (HTTP) + 8081 (TCP 设备)
REM ============================================================

echo ==========================================
echo  IoT Backend Launcher
echo ==========================================
echo.

cd /d D:\iot-java-local-storage-no-db-fixed\iot-java-local-storage-no-db

REM 杀残留 java
taskkill /f /im java.exe >nul 2>nul
timeout /t 2 /nobreak >nul

if not exist pom.xml (
    echo [ERROR] pom.xml not found
    pause
    exit /b 1
)

if not exist target\iot-java-local-storage-1.0.0.jar (
    echo [..] Building...
    call mvn package -DskipTests -q
    if errorlevel 1 (
        echo [ERROR] Build failed
        pause
        exit /b 1
    )
)

echo [..] Starting backend...
echo [INFO] HTTP: http://localhost:8080
echo [INFO] TCP : port 8081
echo.

java -jar target\iot-java-local-storage-1.0.0.jar
pause
