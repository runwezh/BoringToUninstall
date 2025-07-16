@echo off
chcp 65001 > nul
REM BTU 项目构建脚本

echo =====================================
echo   BTU 项目构建工具
echo =====================================
echo.

REM 检查 CMakeLists.txt 是否存在
if not exist "CMakeLists.txt" (
    echo [错误] 未找到 CMakeLists.txt
    echo 请在项目根目录运行此脚本
    pause
    exit /b 1
)

REM 创建构建目录
if not exist "build" (
    echo [信息] 创建构建目录...
    mkdir build
)
cd build

REM 配置项目
echo [1/2] 正在配置项目...
cmake .. -DCMAKE_BUILD_TYPE=Release

if %errorLevel% neq 0 (
    echo [错误] CMake 配置失败
    echo 请检查 Qt 和 CMake 是否正确安装并已添加到 PATH
    pause
    exit /b 1
)

echo [OK] 项目配置成功
echo.

REM 编译项目
echo [2/2] 正在编译项目...
cmake --build . --config Release

if %errorLevel% neq 0 (
    echo [错误] 编译失败
    pause
    exit /b 1
)

echo [OK] 编译成功！
echo.
echo =====================================
echo   构建完成
echo =====================================
echo.
echo 可执行文件位置:
echo %cd%\Release\BTU.exe
echo.
echo 您可以运行 run.bat 来启动程序
echo.
pause
