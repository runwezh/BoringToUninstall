#include "SafetyChecker.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QStandardPaths>

SafetyChecker& SafetyChecker::instance() {
    static SafetyChecker instance;
    return instance;
}

SafetyChecker::SafetyChecker() {
    initializeProtectedPaths();
    initializeProtectedApplications();
    initializeProtectedRegistryKeys();
}

void SafetyChecker::initializeProtectedPaths() {
    // Windows系统关键目录
    m_protectedPaths << "C:\\Windows"
                     << "C:\\Windows\\System32"
                     << "C:\\Windows\\SysWOW64"
                     << "C:\\Windows\\Boot"
                     << "C:\\Windows\\Fonts"
                     << "C:\\Windows\\drivers"
                     << "C:\\Windows\\winsxs"
                     << "C:\\Program Files\\Windows NT"
                     << "C:\\Program Files\\WindowsApps"
                     << "C:\\Program Files\\Common Files\\Microsoft Shared"
                     << "C:\\Program Files (x86)\\Windows NT"
                     << "C:\\Program Files (x86)\\Common Files\\Microsoft Shared"
                     << "C:\\ProgramData\\Microsoft"
                     << "C:\\System Volume Information"
                     << "C:\\$Recycle.Bin"
                     << "C:\\Recovery"
                     << "C:\\Boot"
                     << "C:\\bootmgr"
                     << "C:\\pagefile.sys"
                     << "C:\\hiberfil.sys"
                     << "C:\\swapfile.sys";
}

void SafetyChecker::initializeProtectedApplications() {
    // 系统关键应用程序
    m_protectedApplications << "Microsoft Visual C++"
                           << "Microsoft .NET Framework"
                           << ".NET Framework"
                           << "Windows"
                           << "Microsoft Edge"
                           << "Internet Explorer"
                           << "Windows Media Player"
                           << "Windows Defender"
                           << "Windows Security"
                           << "Microsoft Store"
                           << "Xbox"
                           << "Cortana"
                           << "Windows Photos"
                           << "Windows Camera"
                           << "Windows Calculator"
                           << "Windows Mail"
                           << "Windows Maps"
                           << "Windows Sound Recorder"
                           << "Windows Alarms & Clock"
                           << "Windows Sticky Notes"
                           << "Windows Feedback Hub"
                           << "Windows Get Help"
                           << "Windows Tips"
                           << "Microsoft Solitaire Collection"
                           << "Microsoft Minesweeper";
    
    // 系统发布商
    m_systemPublishers << "Microsoft Corporation"
                      << "Microsoft"
                      << "Windows Software Developer"
                      << "Microsoft Windows"
                      << "Intel Corporation"
                      << "NVIDIA Corporation"
                      << "AMD"
                      << "Realtek Semiconductor Corp."
                      << "Qualcomm Atheros"
                      << "Broadcom"
                      << "Dell Inc."
                      << "HP Inc."
                      << "Lenovo"
                      << "ASUS"
                      << "Acer Incorporated";
}

void SafetyChecker::initializeProtectedRegistryKeys() {
    // 受保护的注册表键
    m_protectedRegistryKeys << "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
                           << "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
                           << "HKEY_LOCAL_MACHINE\\SYSTEM"
                           << "HKEY_LOCAL_MACHINE\\HARDWARE"
                           << "HKEY_LOCAL_MACHINE\\SAM"
                           << "HKEY_LOCAL_MACHINE\\SECURITY"
                           << "HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes"
                           << "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
                           << "HKEY_USERS\\.DEFAULT";
}

bool SafetyChecker::isSafeToDelete(const QString& path) {
    QString normalizedPath = QDir::toNativeSeparators(path.toLower());
    
    // 检查是否为系统关键路径
    if (isSystemCriticalPath(normalizedPath)) {
        LOG_WARNING(QString("阻止删除系统关键路径: %1").arg(path));
        return false;
    }
    
    // 检查是否为Windows系统文件
    if (isWindowsSystemFile(normalizedPath)) {
        LOG_WARNING(QString("阻止删除Windows系统文件: %1").arg(path));
        return false;
    }
    
    // 检查路径长度（防止误删根目录）
    if (path.length() < 10) {
        LOG_WARNING(QString("路径过短，可能为系统根目录: %1").arg(path));
        return false;
    }
    
    return true;
}

bool SafetyChecker::isSystemCriticalPath(const QString& path) {
    QString normalizedPath = QDir::toNativeSeparators(path.toLower());
    
    for (const QString& protectedPath : m_protectedPaths) {
        QString normalizedProtected = QDir::toNativeSeparators(protectedPath.toLower());
        if (normalizedPath.startsWith(normalizedProtected)) {
            return true;
        }
    }
    
    return false;
}

bool SafetyChecker::isWindowsSystemFile(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName().toLower();
    QString dirPath = fileInfo.absolutePath().toLower();
    
    // 检查是否在系统目录中
    QStringList systemDirs = {
        "c:/windows/system32",
        "c:/windows/syswow64",
        "c:/windows/winsxs",
        "c:/windows/drivers"
    };
    
    for (const QString& sysDir : systemDirs) {
        if (dirPath.startsWith(sysDir)) {
            return true;
        }
    }
    
    // 检查关键系统文件
    QStringList systemFiles = {
        "kernel32.dll", "ntdll.dll", "user32.dll", "gdi32.dll",
        "advapi32.dll", "shell32.dll", "ole32.dll", "oleaut32.dll",
        "rpcrt4.dll", "msvcrt.dll", "ws2_32.dll", "wininet.dll",
        "explorer.exe", "winlogon.exe", "csrss.exe", "lsass.exe",
        "services.exe", "svchost.exe", "spoolsv.exe", "dwm.exe"
    };
    
    return systemFiles.contains(fileName);
}

bool SafetyChecker::isSystemApplication(const QString& appName, const QString& publisher) {
    // 检查应用程序名称
    for (const QString& protectedApp : m_protectedApplications) {
        if (appName.contains(protectedApp, Qt::CaseInsensitive)) {
            return true;
        }
    }
    
    // 检查发布商
    for (const QString& systemPublisher : m_systemPublishers) {
        if (publisher.contains(systemPublisher, Qt::CaseInsensitive)) {
            return true;
        }
    }
    
    return false;
}

bool SafetyChecker::isSafeRegistryKey(const QString& keyPath) {
    QString normalizedKey = keyPath.toUpper();
    
    for (const QString& protectedKey : m_protectedRegistryKeys) {
        if (normalizedKey.startsWith(protectedKey.toUpper())) {
            return false;
        }
    }
    
    return true;
}

QStringList SafetyChecker::getProtectedPaths() const {
    return m_protectedPaths;
}

QStringList SafetyChecker::getProtectedApplications() const {
    return m_protectedApplications;
}
