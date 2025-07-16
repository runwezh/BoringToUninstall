#include "AppScanner.h"
#include "SafetyChecker.h"
#include "Logger.h"
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winreg.h>
#endif

AppScanner::AppScanner(QObject* parent)
    : QObject(parent)
    , m_scanThread(nullptr)
    , m_isScanning(false)
    , m_shouldStop(false)
{
}

AppScanner::~AppScanner() {
    stopScan();
}

void AppScanner::startScan() {
    if (m_isScanning) {
        return;
    }
    
    m_isScanning = true;
    m_shouldStop = false;
    m_applications.clear();
    
    // 创建工作线程
    m_scanThread = new QThread(this);
    ScanWorker* worker = new ScanWorker(this);
    worker->moveToThread(m_scanThread);
    
    // 连接信号
    connect(m_scanThread, &QThread::started, worker, &ScanWorker::doWork);
    connect(worker, &ScanWorker::finished, this, &AppScanner::onScanFinished);
    connect(worker, &ScanWorker::applicationFound, this, &AppScanner::applicationFound);
    connect(worker, &ScanWorker::progress, this, &AppScanner::scanProgress);
    connect(worker, &ScanWorker::error, this, &AppScanner::scanError);
    
    emit scanStarted();
    LOG_INFO("开始扫描已安装应用程序");
    
    m_scanThread->start();
}

void AppScanner::stopScan() {
    if (!m_isScanning) {
        return;
    }
    
    m_shouldStop = true;
    
    if (m_scanThread) {
        m_scanThread->quit();
        m_scanThread->wait(5000); // 等待最多5秒
        m_scanThread->deleteLater();
        m_scanThread = nullptr;
    }
    
    m_isScanning = false;
}

void AppScanner::onScanFinished() {
    m_isScanning = false;
    
    if (m_scanThread) {
        m_scanThread->quit();
        m_scanThread->wait();
        m_scanThread->deleteLater();
        m_scanThread = nullptr;
    }
    
    LOG_INFO(QString("扫描完成，找到 %1 个应用程序").arg(m_applications.size()));
    emit scanFinished();
}

QList<ApplicationInfo> AppScanner::getApplications() const {
    QMutexLocker locker(&m_mutex);
    return m_applications;
}

QList<ApplicationInfo> AppScanner::searchApplications(const QString& keyword) const {
    QMutexLocker locker(&m_mutex);
    QList<ApplicationInfo> results;
    
    QString lowerKeyword = keyword.toLower();
    
    for (const ApplicationInfo& app : m_applications) {
        if (app.displayName.toLower().contains(lowerKeyword) ||
            app.name.toLower().contains(lowerKeyword) ||
            app.publisher.toLower().contains(lowerKeyword)) {
            results.append(app);
        }
    }
    
    return results;
}

void AppScanner::refreshApplications() {
    stopScan();
    startScan();
}

bool AppScanner::isScanning() const {
    return m_isScanning;
}

QString AppScanner::formatSize(qint64 bytes) const {
    if (bytes < 1024) {
        return QString("%1 B").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        return QString("%1 KB").arg(bytes / 1024);
    } else if (bytes < 1024 * 1024 * 1024) {
        return QString("%1 MB").arg(bytes / (1024 * 1024));
    } else {
        return QString("%1 GB").arg(bytes / (1024 * 1024 * 1024));
    }
}

QString AppScanner::parseInstallDate(const QString& dateStr) const {
    if (dateStr.length() == 8) {
        // YYYYMMDD格式
        QString year = dateStr.mid(0, 4);
        QString month = dateStr.mid(4, 2);
        QString day = dateStr.mid(6, 2);
        return QString("%1-%2-%3").arg(year, month, day);
    }
    return dateStr;
}

// ScanWorker实现
ScanWorker::ScanWorker(AppScanner* scanner)
    : m_scanner(scanner)
{
}

void ScanWorker::doWork() {
    try {
        QStringList registryKeys = {
            "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
            "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
            "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
        };
        
        int totalProcessed = 0;
        int totalKeys = 0;
        
        // 首先计算总数
        for (const QString& keyPath : registryKeys) {
            QSettings registry(keyPath, QSettings::NativeFormat);
            totalKeys += registry.childGroups().size();
        }
        
        emit progress(0, totalKeys);
        
        SafetyChecker& safety = SafetyChecker::instance();
        
        for (const QString& keyPath : registryKeys) {
            QSettings registry(keyPath, QSettings::NativeFormat);
            QStringList subKeys = registry.childGroups();
            
            for (const QString& subKey : subKeys) {
                if (m_scanner->m_shouldStop) {
                    break;
                }
                
                registry.beginGroup(subKey);
                
                ApplicationInfo appInfo;
                appInfo.name = registry.value("DisplayName").toString();
                appInfo.displayName = appInfo.name;
                appInfo.version = registry.value("DisplayVersion").toString();
                appInfo.publisher = registry.value("Publisher").toString();
                appInfo.installDate = m_scanner->parseInstallDate(registry.value("InstallDate").toString());
                appInfo.installLocation = registry.value("InstallLocation").toString();
                appInfo.uninstallString = registry.value("UninstallString").toString();
                appInfo.registryKey = keyPath + "\\" + subKey;
                
                // 处理估算大小
                QVariant sizeVar = registry.value("EstimatedSize");
                if (sizeVar.isValid()) {
                    qint64 sizeKB = sizeVar.toLongLong();
                    appInfo.estimatedSize = m_scanner->formatSize(sizeKB * 1024);
                } else {
                    appInfo.estimatedSize = "未知";
                }
                
                registry.endGroup();
                
                // 检查是否为有效应用
                if (!appInfo.name.isEmpty() && !appInfo.uninstallString.isEmpty()) {
                    // 检查是否为系统应用
                    appInfo.isSystemApp = safety.isSystemApplication(appInfo.name, appInfo.publisher);
                    appInfo.canUninstall = !appInfo.isSystemApp;
                    
                    QMutexLocker locker(&m_scanner->m_mutex);
                    m_scanner->m_applications.append(appInfo);
                    
                    emit applicationFound(appInfo);
                }
                
                totalProcessed++;
                emit progress(totalProcessed, totalKeys);
            }
            
            if (m_scanner->m_shouldStop) {
                break;
            }
        }
        
    } catch (const std::exception& e) {
        emit error(QString("扫描过程中发生错误: %1").arg(e.what()));
    } catch (...) {
        emit error("扫描过程中发生未知错误");
    }
    
    emit finished();
}

#include "AppScanner.moc"
