@echo off
REM BTU 开发环境自动化安装脚本
REM 适用于 Windows 10/11 + Visual Studio 2022

echo =====================================
echo   BTU 开发环境自动化安装工具
echo =====================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% == 0 (
    echo [OK] 检测到管理员权限
) else (
    echo [错误] 需要管理员权限运行此脚本
    echo 请右键选择「以管理员身份运行」
    pause
    exit /b 1
)

echo.
echo 正在检查系统环境...

REM 检查 Visual Studio 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022" (
    echo [OK] 检测到 Visual Studio 2022
) else (
    echo [警告] 未检测到 Visual Studio 2022
    echo 请确保已安装 Visual Studio 2022 包含 C++ 开发工具
)

REM 检查系统架构
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    echo [OK] 检测到 64 位系统
) else (
    echo [错误] 需要 64 位系统
    pause
    exit /b 1
)

echo.
echo 开始安装依赖组件...
echo.

REM 安装 CMake
echo [1/3] 正在检查 CMake...
cmake --version >nul 2>&1
if %errorLevel% == 0 (
    echo [OK] CMake 已安装
) else (
    echo [信息] 正在下载 CMake...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = 'Tls12'; Invoke-WebRequest -Uri 'https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-windows-x86_64.msi' -OutFile 'cmake_installer.msi'"
    
    if exist "cmake_installer.msi" (
        echo [信息] 正在安装 CMake...
        msiexec /i cmake_installer.msi /quiet ADD_CMAKE_TO_PATH=System
        echo [OK] CMake 安装完成
        del cmake_installer.msi
    ) else (
        echo [错误] CMake 下载失败，请手动安装
    )
)

echo.
echo [2/3] 正在检查 Git...
git --version >nul 2>&1
if %errorLevel% == 0 (
    echo [OK] Git 已安装
) else (
    echo [信息] 正在下载 Git...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = 'Tls12'; Invoke-WebRequest -Uri 'https://github.com/git-for-windows/git/releases/download/v2.43.0.windows.1/Git-2.43.0-64-bit.exe' -OutFile 'git_installer.exe'"
    
    if exist "git_installer.exe" (
        echo [信息] 正在安装 Git...
        git_installer.exe /VERYSILENT /NORESTART
        echo [OK] Git 安装完成
        del git_installer.exe
    ) else (
        echo [错误] Git 下载失败，请手动安装
    )
)

echo.
echo [3/3] 正在检查 Qt...
if exist "C:\Qt" (
    echo [OK] 检测到 Qt 安装目录
    
    REM 检查具体版本
    if exist "C:\Qt\6.*\msvc2022_64" (
        echo [OK] 检测到 Qt 6.x + MSVC2022
    ) else (
        echo [警告] 未检测到兼容的 Qt 版本
        goto qt_install_guide
    )
) else (
    :qt_install_guide
    echo [信息] 未检测到 Qt 安装
    echo.
    echo Qt 需要手动安装，请按照以下步骤：
    echo 1. 访问 https://www.qt.io/download
    echo 2. 创建免费账户
    echo 3. 下载 Qt Online Installer
    echo 4. 安装 Qt 6.8 LTS + MSVC 2022 64-bit
    echo 5. 安装路径选择：C:\Qt
    echo.
    echo 详细步骤请参考 ENVIRONMENT_SETUP.md 文档
)

echo.
echo =====================================
echo   环境配置
echo =====================================

REM 设置环境变量
echo 正在配置环境变量...

if exist "C:\Qt" (
    for /d %%i in ("C:\Qt\6.*") do (
        if exist "%%i\msvc2022_64\bin" (
            echo 添加 Qt 路径到 PATH: %%i\msvc2022_64\bin
            setx PATH "%PATH%;%%i\msvc2022_64\bin" /M >nul
            setx CMAKE_PREFIX_PATH "%%i\msvc2022_64" /M >nul
        )
    )
)

echo.
echo =====================================
echo   项目构建测试
echo =====================================

if exist "CMakeLists.txt" (
    echo 检测到 BTU 项目，开始测试构建...
    
    REM 创建构建目录
    if not exist "build" mkdir build
    cd build
    
    REM 配置项目
    echo 正在配置项目...
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    if %errorLevel% == 0 (
        echo [OK] 项目配置成功
        
        REM 尝试编译
        echo 正在编译项目...
        cmake --build . --config Release
        
        if %errorLevel% == 0 (
            echo [OK] 编译成功！
            echo 可执行文件位置：build\Release\BTU.exe
        ) else (
            echo [警告] 编译失败，可能需要安装 Qt
        )
    ) else (
        echo [警告] 项目配置失败，请检查 Qt 安装
    )
    
    cd ..
) else (
    echo 未检测到 BTU 项目文件
    echo 请在项目根目录运行此脚本
)

echo.
echo =====================================
echo   安装完成
echo =====================================
echo.
echo 环境配置状态：
cmake --version >nul 2>&1 && echo [OK] CMake 可用 || echo [错误] CMake 不可用
git --version >nul 2>&1 && echo [OK] Git 可用 || echo [错误] Git 不可用

if exist "C:\Qt" (
    echo [OK] Qt 安装目录存在
) else (
    echo [待办] 需要手动安装 Qt
)

echo.
echo 下一步：
echo 1. 如果 Qt 未安装，请参考 ENVIRONMENT_SETUP.md 手动安装
echo 2. 重启命令提示符以刷新环境变量
echo 3. 运行：cmake --build build --config Release
echo.
echo 详细文档：ENVIRONMENT_SETUP.md
echo 项目地址：https://github.com/runwezh/BoringToUninstall
echo.
pause
