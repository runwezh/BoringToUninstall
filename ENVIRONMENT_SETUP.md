# BTU 开发环境配置手册

## 概述
本手册将指导您在 Windows 系统上设置 BoringToUninstall (BTU) 项目的完整开发环境。

## 环境检查清单

### 系统要求
- Windows 10 或更高版本
- 64位操作系统  
- 至少 8GB 内存
- 至少 10GB 可用磁盘空间
- 管理员权限

### 已有环境
- [x] Visual Studio 2022 (已安装)
- [ ] Qt 6.x 开发框架
- [ ] CMake 3.16+

## 详细安装步骤

### 第一步：安装 CMake

1. **下载 CMake**
   - 访问：https://cmake.org/download/
   - 下载 Windows x64 Installer 版本
   - 选择最新稳定版本 (推荐 3.28+)

2. **安装 CMake**
   - 运行下载的安装程序
   - 勾选 Add CMake to the system PATH for all users
   - 完成安装后重启命令提示符

3. **验证安装**
   `
   cmake --version
   `

### 第二步：安装 Qt 6.x 开发框架

#### Qt 新手必读

**什么是 Qt？**
- Qt 是一个跨平台的 C++ 图形用户界面应用程序开发框架
- BTU 项目使用 Qt 来创建现代化的 Windows 桌面界面  
- Qt 提供了丰富的 GUI 组件和开发工具

**Qt 主要组件：**
- Qt Creator: 集成开发环境 (IDE)
- Qt Designer: 可视化界面设计工具
- Qt Libraries: C++ 类库
- qmake/CMake: 构建系统

#### 安装步骤

1. **创建 Qt 账户**
   - 访问：https://www.qt.io/download
   - 点击 Go open source 创建免费账户
   - 验证邮箱地址

2. **下载 Qt Online Installer**
   - 选择 Qt Online Installer for Windows
   - 下载并运行安装程序

3. **安装配置**
   - 登录您的 Qt 账户
   - 选择 Custom installation
   - 推荐安装组件：
     * Qt 6.8 LTS (或最新LTS版本)
     * MSVC 2022 64-bit
     * Qt Creator
     * CMake
     * Ninja

4. **选择安装路径**
   - 推荐安装到：C:\Qt
   - 确保路径中没有空格和中文字符

5. **完成安装**
   - 安装过程可能需要30-60分钟
   - 确保网络连接稳定

#### Qt 安装验证

打开命令提示符，验证安装：
`
C:\Qt\6.8.1\msvc2022_64\bin\qmake --version
`

### 第三步：配置环境变量

1. **打开环境变量设置**
   - 按 Win+R，输入 sysdm.cpl
   - 点击 高级 -> 环境变量

2. **添加 Qt 路径**
   在系统变量 PATH 中添加：
   `
   C:\Qt\6.8.1\msvc2022_64\bin
   C:\Qt\Tools\CMake_64\bin
   `

3. **设置 CMAKE_PREFIX_PATH** (可选)
   创建新的系统环境变量：
   - 变量名：CMAKE_PREFIX_PATH
   - 变量值：C:\Qt\6.8.1\msvc2022_64

### 第四步：配置 Visual Studio 2022

1. **安装 Qt Visual Studio Tools**
   - 打开 Visual Studio 2022
   - 转到 扩展 -> 管理扩展
   - 搜索 Qt Visual Studio Tools
   - 下载并安装

2. **配置 Qt 版本**
   - 重启 Visual Studio
   - 转到 扩展 -> Qt VS Tools -> Qt Options
   - 添加 Qt 版本：C:\Qt\6.8.1\msvc2022_64

## BTU 项目构建

### 克隆项目

`ash
git clone https://github.com/runwezh/BoringToUninstall.git
cd BoringToUninstall
`

### 构建项目

1. **创建构建目录**
   `cmd
   mkdir build
   cd build
   `

2. **配置项目**
   `cmd
   cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.8.1\msvc2022_64
   `

3. **编译项目**
   `cmd
   cmake --build . --config Release
   `

4. **运行程序**
   `cmd
   cd Release
   BTU.exe
   `

## 常见问题解决

### Qt 安装问题

**问题：无法下载 Qt 组件**
- 解决：检查网络连接，尝试使用 VPN
- 或者下载离线安装包

**问题：找不到 MSVC 编译器**
- 解决：确保安装了 Visual Studio 2022 C++ 工具集
- 在 Visual Studio Installer 中添加 MSVC v143 编译器工具集

### CMake 配置问题

**问题：CMake 找不到 Qt**
- 解决：设置 CMAKE_PREFIX_PATH 环境变量
- 或在 cmake 命令中指定路径

**问题：编译错误 - 找不到头文件**
- 解决：检查 Qt 安装是否完整
- 重新运行 cmake 配置

### 运行时问题

**问题：程序启动时缺少 DLL**
- 解决：将 Qt\6.8.1\msvc2022_64\bin 添加到 PATH
- 或者复制所需 DLL 到程序目录

## 开发工具推荐

### 代码编辑器
- **Visual Studio 2022** (主要IDE)
- **Qt Creator** (Qt 专用IDE)
- **Visual Studio Code** (轻量级编辑器)

### 调试工具
- Visual Studio 调试器
- Qt Creator 调试器
- Application Verifier (内存检查)

### 版本控制
- Git for Windows
- GitHub Desktop
- Visual Studio Git 集成

## 下一步

环境配置完成后，您可以：

1. **学习 Qt 基础**
   - 阅读 Qt 官方文档
   - 完成 Qt 入门教程
   - 理解信号和槽机制

2. **熟悉项目结构**
   - 查看 src/ 目录下的源代码
   - 理解各个模块的功能
   - 学习 CMake 构建配置

3. **开始开发**
   - 修改现有功能
   - 添加新特性
   - 编写测试代码

---

**需要帮助？**
如果在环境配置过程中遇到问题，可以：
- 查看本手册的常见问题部分
- 查阅 Qt 官方文档
- 在项目 GitHub 页面提交 Issue

