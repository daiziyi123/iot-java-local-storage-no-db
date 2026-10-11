@echo off
REM ============================================================
REM  一键发送 JSON 给板子（CMD 版）
REM  实际调用 PowerShell 脚本 (JSON 处理更干净)
REM  用法:
REM    send_to_board.bat                  发1次 message=5678
REM    send_to_board.bat 9999             发 message=9999
REM    send_to_board.bat 5678 10          发10次
REM    send_to_board.bat 5678 0 3         无限循环, 间隔3秒
REM ============================================================

setlocal
set "MESSAGE=%~1"
if "%MESSAGE%"=="" set "MESSAGE=5678"

set "MAXSEND=%~2"
if "%MAXSEND%"=="" set "MAXSEND=1"

set "INTERVAL=%~3"
if "%INTERVAL%"=="" set "INTERVAL=3"

REM 调 PowerShell 脚本
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\send_to_board.ps1" ^
    -Message "%MESSAGE%" ^
    -MaxSend %MAXSEND% ^
    -IntervalSec %INTERVAL%

endlocal
pause
