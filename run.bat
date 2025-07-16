@echo off
REM BTU 项目运行脚本

echo =====================================
echo   BTU 项目运行工具
echo =====================================
echo.

set EXECUTABLE_PATH=build\Release\BTU.exe

REM 检查可执行文件是否存在
if not exist "%EXECUTABLE_PATH%" (
    echo [错误] 未找到可执行文件: %EXECUTABLE_PATH%
    echo.
    echo 请先运行 build.bat 来编译项目
    pause
    exit /b 1
)

echo [信息] 正在启动 BTU...
echo 可执行文件: %EXECUTABLE_PATH%
echo.

REM 启动程序
start "" "%EXECUTABLE_PATH%"

exit
