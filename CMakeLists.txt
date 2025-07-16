cmake_minimum_required(VERSION 3.16)
project(BoringToUninstall VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 设置Qt自动化工具
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(src)

# 源文件
set(SOURCES
    src/main.cpp
    src/BTUMainWindow.cpp
    src/AppScanner.cpp
    src/UninstallEngine.cpp
    src/SafetyChecker.cpp
    src/Logger.cpp
)

# 头文件
set(HEADERS
    src/BTUMainWindow.h
    src/AppScanner.h
    src/UninstallEngine.h
    src/SafetyChecker.h
    src/Logger.h
    src/Version.h
)

# UI文件
set(UI_FILES
    ui/btu_mainwindow.ui
)

# 资源文件
set(RESOURCE_FILES
    resources/btu_resources.qrc
)

# Windows下设置图标
if(WIN32)
    set(SOURCES ${SOURCES} resources/btu.rc)
endif()

# 创建可执行文件
add_executable(BTU WIN32 ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCE_FILES})

# 链接Qt库
target_link_libraries(BTU Qt6::Core Qt6::Widgets)

# Windows特定库
if(WIN32)
    target_link_libraries(BTU advapi32 shell32 ole32 user32)
endif()

# 设置输出目录
set_target_properties(BTU PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release
)

# 安装配置
install(TARGETS BTU
    DESTINATION bin
)
