#include "BTUMainWindow.h"
#include "Logger.h"
#include "Version.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QTranslator>
#include <QLocale>

#ifdef Q_OS_WIN
#include <windows.h>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

// 检查是否已有实例在运行
bool isAlreadyRunning() {
#ifdef Q_OS_WIN
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "BTU_BoringToUninstall_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return true;
    }
#endif
    return false;
}

// 检查管理员权限
bool isRunAsAdmin() {
#ifdef Q_OS_WIN
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;
    
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                &administratorsGroup)) {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }
    
    return isAdmin == TRUE;
#else
    return false;
#endif
}

// 请求管理员权限
bool requestAdminPrivileges() {
#ifdef Q_OS_WIN
    if (isRunAsAdmin()) {
        return true;
    }
    
    int result = QMessageBox::question(nullptr, "权限提示",
        "BTU需要管理员权限才能正常工作。\n"
        "这是为了能够访问系统注册表和删除受保护的文件。\n\n"
        "是否要以管理员身份重新启动？",
        QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        QString programPath = QApplication::applicationFilePath();
        
        SHELLEXECUTEINFOA shExInfo = {0};
        shExInfo.cbSize = sizeof(shExInfo);
        shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        shExInfo.hwnd = NULL;
        shExInfo.lpVerb = "runas";
        shExInfo.lpFile = programPath.toLocal8Bit().data();
        shExInfo.lpParameters = NULL;
        shExInfo.lpDirectory = NULL;
        shExInfo.nShow = SW_SHOW;
        shExInfo.hInstApp = NULL;
        
        if (ShellExecuteExA(&shExInfo)) {
            return false; // 返回false表示需要退出当前进程
        }
    }
    
    return false;
#else
    return true;
#endif
}

// 设置应用程序样式
void setupApplicationStyle(QApplication& app) {
    // 设置应用程序样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 设置现代化的调色板
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    palette.setColor(QPalette::Text, QColor(0, 0, 0));
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    
    app.setPalette(palette);
    
    // 设置全局样式表
    QString globalStyle = R"(
        QMainWindow {
            background-color: #f0f0f0;
        }
        
        QMenuBar {
            background-color: #ffffff;
            border-bottom: 1px solid #ddd;
            padding: 2px;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
            border-radius: 4px;
        }
        
        QMenuBar::item:selected {
            background-color: #e3f2fd;
        }
        
        QMenu {
            background-color: #ffffff;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 4px;
        }
        
        QMenu::item {
            padding: 6px 20px;
            border-radius: 4px;
        }
        
        QMenu::item:selected {
            background-color: #2196f3;
            color: white;
        }
        
        QStatusBar {
            background-color: #f5f5f5;
            border-top: 1px solid #ddd;
        }
        
        QMessageBox {
            background-color: #ffffff;
        }
        
        QDialog {
            background-color: #f0f0f0;
        }
        
        QScrollBar:vertical {
            background-color: #f0f0f0;
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #c0c0c0;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #a0a0a0;
        }
    )";
    
    app.setStyleSheet(globalStyle);
}

// 创建启动画面
QSplashScreen* createSplashScreen() {
    // 创建一个简单的启动画面
    QPixmap pixmap(400, 300);
    pixmap.fill(QColor(33, 150, 243));
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景渐变
    QLinearGradient gradient(0, 0, 0, 300);
    gradient.setColorAt(0, QColor(33, 150, 243));
    gradient.setColorAt(1, QColor(21, 101, 192));
    painter.fillRect(pixmap.rect(), gradient);
    
    // 绘制应用程序名称
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 24, QFont::Bold));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "BoringToUninstall");
    
    // 绘制版本信息
    painter.setFont(QFont("Arial", 12));
    painter.drawText(QRect(0, 250, 400, 30), Qt::AlignCenter, 
                    QString("版本 %1 - 让卸载不再无聊").arg(BTU_VERSION_STRING));
    
    // 绘制版权信息
    painter.setFont(QFont("Arial", 10));
    painter.drawText(QRect(0, 270, 400, 20), Qt::AlignCenter, BTU_APP_COPYRIGHT);
    
    painter.end();
    
    QSplashScreen* splash = new QSplashScreen(pixmap);
    splash->setWindowFlag(Qt::WindowStaysOnTopHint);
    
    return splash;
}

// 初始化应用程序目录
void initializeAppDirectories() {
    QStringList dirs = {
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups"
    };
    
    for (const QString& dir : dirs) {
        QDir().mkpath(dir);
    }
}

int main(int argc, char *argv[])
{
    // 设置应用程序属性
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName(BTU_APP_NAME);
    app.setApplicationDisplayName(BTU_APP_DISPLAY_NAME);
    app.setApplicationVersion(BTU_VERSION_STRING);
    app.setOrganizationName(BTU_APP_COMPANY);
    app.setOrganizationDomain("btu.local");
    
    // 检查是否已有实例在运行
    if (isAlreadyRunning()) {
        QMessageBox::warning(nullptr, "程序已运行", 
            "BoringToUninstall 已经在运行中。\n请检查系统托盘或任务栏。");
        return 1;
    }
    
    // 检查管理员权限
    if (!requestAdminPrivileges()) {
        return 0; // 用户选择以管理员身份重启，退出当前进程
    }
    
    // 如果不是管理员权限，显示警告但继续运行
    if (!isRunAsAdmin()) {
        QMessageBox::warning(nullptr, "权限警告",
            "程序未以管理员身份运行。\n"
            "某些功能可能受到限制，建议以管理员身份运行以获得最佳体验。");
    }
    
    // 初始化应用程序目录
    initializeAppDirectories();
    
    // 初始化日志系统
    LOG_INFO("=== BoringToUninstall 启动 ===");
    LOG_INFO(QString("版本: %1").arg(BTU_VERSION_STRING));
    LOG_INFO(QString("管理员权限: %1").arg(isRunAsAdmin() ? "是" : "否"));
    
    // 设置应用程序样式
    setupApplicationStyle(app);
    
    // 创建并显示启动画面
    QSplashScreen* splash = createSplashScreen();
    splash->show();
    app.processEvents();
    
    // 启动画面显示消息
    splash->showMessage("正在初始化...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();
    
    // 模拟初始化延迟
    QTimer::singleShot(1000, [splash]() {
        splash->showMessage("正在加载组件...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    });
    
    QTimer::singleShot(2000, [splash]() {
        splash->showMessage("准备就绪", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    });
    
    // 创建主窗口
    BTUMainWindow mainWindow;
    
    // 3秒后关闭启动画面并显示主窗口
    QTimer::singleShot(3000, [&mainWindow, splash]() {
        splash->finish(&mainWindow);
        mainWindow.show();
        splash->deleteLater();
    });
    
    LOG_INFO("主窗口创建完成，进入事件循环");
    
    // 进入事件循环
    int result = app.exec();
    
    LOG_INFO("=== BoringToUninstall 退出 ===");
    
    return result;
}
