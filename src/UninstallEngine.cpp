#include "UninstallEngine.h"
#include "SafetyChecker.h"
#include "Logger.h"
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>
#include <QApplication>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winreg.h>
#include <shellapi.h>
#include <shlobj.h>
#endif

UninstallEngine::UninstallEngine(QObject* parent)
    : QObject(parent)
    , m_uninstallThread(nullptr)
    , m_isUninstalling(false)
    , m_shouldStop(false)
    , m_createBackup(false)
    , m_forceDelete(false)
    , m_currentIndex(0)
{
}

UninstallEngine::~UninstallEngine() {
    stopUninstall();
}

void UninstallEngine::uninstallApplication(const ApplicationInfo& appInfo) {
    QList<ApplicationInfo> appList;
    appList.append(appInfo);
    uninstallApplications(appList);
}

void UninstallEngine::uninstallApplications(const QList<ApplicationInfo>& appList) {
    if (m_isUninstalling) {
        return;
    }
    
    m_isUninstalling = true;
    m_shouldStop = false;
    m_uninstallQueue = appList;
    m_currentIndex = 0;
    
    // 创建工作线程
    m_uninstallThread = new QThread(this);
    UninstallWorker* worker = new UninstallWorker(this, appList);
    worker->moveToThread(m_uninstallThread);
    
    // 连接信号
    connect(m_uninstallThread, &QThread::started, worker, &UninstallWorker::doWork);
    connect(worker, &UninstallWorker::finished, this, &UninstallEngine::onUninstallFinished);
    connect(worker, &UninstallWorker::uninstallStarted, this, &UninstallEngine::uninstallStarted);
    connect(worker, &UninstallWorker::uninstallFinished, this, &UninstallEngine::uninstallFinished);
    connect(worker, &UninstallWorker::uninstallProgress, this, &UninstallEngine::uninstallProgress);
    connect(worker, &UninstallWorker::uninstallError, this, &UninstallEngine::uninstallError);
    
    LOG_INFO(QString("开始批量卸载 %1 个应用程序").arg(appList.size()));
    
    m_uninstallThread->start();
}

void UninstallEngine::stopUninstall() {
    if (!m_isUninstalling) {
        return;
    }
    
    m_shouldStop = true;
    
    if (m_uninstallThread) {
        m_uninstallThread->quit();
        m_uninstallThread->wait(10000); // 等待最多10秒
        m_uninstallThread->deleteLater();
        m_uninstallThread = nullptr;
    }
    
    m_isUninstalling = false;
}

void UninstallEngine::onUninstallFinished() {
    m_isUninstalling = false;
    
    if (m_uninstallThread) {
        m_uninstallThread->quit();
        m_uninstallThread->wait();
        m_uninstallThread->deleteLater();
        m_uninstallThread = nullptr;
    }
    
    LOG_INFO("批量卸载完成");
    emit allUninstallsFinished();
}

bool UninstallEngine::isUninstalling() const {
    return m_isUninstalling;
}

void UninstallEngine::setCreateBackup(bool enabled) {
    m_createBackup = enabled;
}

void UninstallEngine::setForceDelete(bool enabled) {
    m_forceDelete = enabled;
}

bool UninstallEngine::runNativeUninstaller(const ApplicationInfo& appInfo) {
    QString uninstallCmd = appInfo.uninstallString;
    if (uninstallCmd.isEmpty()) {
        return false;
    }
    
    LOG_INFO(QString("运行原生卸载程序: %1").arg(uninstallCmd));
    
    // 解析卸载命令
    QProcess process;
    process.setProgram("cmd.exe");
    QStringList arguments;
    arguments << "/c" << uninstallCmd;
    process.setArguments(arguments);
    
    // 启动卸载进程
    process.start();
    if (!process.waitForStarted(5000)) {
        LOG_ERROR(QString("无法启动卸载程序: %1").arg(process.errorString()));
        return false;
    }
    
    // 等待卸载完成（最多等待5分钟）
    if (!process.waitForFinished(300000)) {
        LOG_ERROR("卸载程序超时");
        process.kill();
        return false;
    }
    
    int exitCode = process.exitCode();
    LOG_INFO(QString("卸载程序退出码: %1").arg(exitCode));
    
    return exitCode == 0;
}

bool UninstallEngine::deleteDirectory(const QString& dirPath) {
    SafetyChecker& safety = SafetyChecker::instance();
    
    if (!safety.isSafeToDelete(dirPath)) {
        LOG_WARNING(QString("安全检查失败，跳过删除目录: %1").arg(dirPath));
        return false;
    }
    
    QDir dir(dirPath);
    if (!dir.exists()) {
        return true; // 目录不存在，认为删除成功
    }
    
    LOG_INFO(QString("删除目录: %1").arg(dirPath));
    
    // 递归删除目录
    bool success = dir.removeRecursively();
    if (!success) {
        LOG_ERROR(QString("删除目录失败: %1").arg(dirPath));
    }
    
    return success;
}

bool UninstallEngine::deleteRegistryKeys(const ApplicationInfo& appInfo) {
    SafetyChecker& safety = SafetyChecker::instance();
    
    if (!safety.isSafeRegistryKey(appInfo.registryKey)) {
        LOG_WARNING(QString("注册表键不安全，跳过删除: %1").arg(appInfo.registryKey));
        return false;
    }
    
    LOG_INFO(QString("删除注册表键: %1").arg(appInfo.registryKey));
    
    QSettings registry(appInfo.registryKey, QSettings::NativeFormat);
    registry.clear();
    registry.sync();
    
    return true;
}

bool UninstallEngine::cleanUserData(const ApplicationInfo& appInfo) {
    QStringList userDataPaths;
    
    // 常见的用户数据路径
    QString appDataLocal = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString appDataRoaming = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    
    // 基于应用名称查找相关目录
    QString appName = appInfo.name;
    QString publisher = appInfo.publisher;
    
    QStringList possibleNames = {appName, publisher};
    
    for (const QString& name : possibleNames) {
        if (name.isEmpty()) continue;
        
        userDataPaths << QString("%1/%2").arg(appDataLocal, name);
        userDataPaths << QString("%1/%2").arg(appDataRoaming, name);
        userDataPaths << QString("%1/%2").arg(documents, name);
    }
    
    bool success = true;
    for (const QString& path : userDataPaths) {
        QDir dir(path);
        if (dir.exists()) {
            LOG_INFO(QString("清理用户数据: %1").arg(path));
            if (!deleteDirectory(path)) {
                success = false;
            }
        }
    }
    
    return success;
}

bool UninstallEngine::cleanTemporaryFiles(const ApplicationInfo& appInfo) {
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir tempDir(tempPath);
    
    // 查找与应用相关的临时文件
    QStringList nameFilters;
    nameFilters << QString("*%1*").arg(appInfo.name.split(" ").first());
    
    QFileInfoList tempFiles = tempDir.entryInfoList(nameFilters, QDir::Files | QDir::Dirs);
    
    bool success = true;
    for (const QFileInfo& fileInfo : tempFiles) {
        if (fileInfo.isDir()) {
            if (!deleteDirectory(fileInfo.absoluteFilePath())) {
                success = false;
            }
        } else {
            QFile file(fileInfo.absoluteFilePath());
            if (!file.remove()) {
                LOG_WARNING(QString("删除临时文件失败: %1").arg(fileInfo.absoluteFilePath()));
                success = false;
            }
        }
    }
    
    return success;
}

bool UninstallEngine::cleanStartupEntries(const ApplicationInfo& appInfo) {
    QStringList startupKeys = {
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
    };
    
    bool found = false;
    
    for (const QString& keyPath : startupKeys) {
        QSettings registry(keyPath, QSettings::NativeFormat);
        QStringList keys = registry.allKeys();
        
        for (const QString& key : keys) {
            QString value = registry.value(key).toString();
            if (value.contains(appInfo.name, Qt::CaseInsensitive) ||
                (!appInfo.installLocation.isEmpty() && value.contains(appInfo.installLocation, Qt::CaseInsensitive))) {
                
                LOG_INFO(QString("删除启动项: %1").arg(key));
                registry.remove(key);
                found = true;
            }
        }
    }
    
    return found;
}

bool UninstallEngine::cleanServices(const ApplicationInfo& appInfo) {
    // 服务清理需要更谨慎，这里只是一个基本实现
    // 实际应用中可能需要更复杂的逻辑
    LOG_INFO(QString("检查与 %1 相关的服务").arg(appInfo.name));
    
    // 这里可以添加服务检查和删除的逻辑
    // 由于安全考虑，暂时不实现自动删除服务的功能
    
    return true;
}

bool UninstallEngine::performDeepClean(const ApplicationInfo& appInfo) {
    bool success = true;
    
    UninstallProgress progress;
    progress.currentOperation = "深度清理中...";
    emit uninstallProgress(appInfo.name, progress);
    
    // 1. 删除安装目录
    if (!appInfo.installLocation.isEmpty()) {
        progress.currentOperation = "删除安装目录...";
        emit uninstallProgress(appInfo.name, progress);
        
        if (!deleteDirectory(appInfo.installLocation)) {
            success = false;
        }
    }
    
    // 2. 删除注册表项
    progress.currentOperation = "清理注册表...";
    emit uninstallProgress(appInfo.name, progress);
    
    if (!deleteRegistryKeys(appInfo)) {
        success = false;
    }
    
    // 3. 清理用户数据
    progress.currentOperation = "清理用户数据...";
    emit uninstallProgress(appInfo.name, progress);
    
    if (!cleanUserData(appInfo)) {
        success = false;
    }
    
    // 4. 清理临时文件
    progress.currentOperation = "清理临时文件...";
    emit uninstallProgress(appInfo.name, progress);
    
    cleanTemporaryFiles(appInfo);
    
    // 5. 清理启动项
    progress.currentOperation = "清理启动项...";
    emit uninstallProgress(appInfo.name, progress);
    
    cleanStartupEntries(appInfo);
    
    // 6. 检查服务
    progress.currentOperation = "检查服务...";
    emit uninstallProgress(appInfo.name, progress);
    
    cleanServices(appInfo);
    
    progress.isComplete = true;
    progress.currentOperation = "清理完成";
    emit uninstallProgress(appInfo.name, progress);
    
    return success;
}

// UninstallWorker实现
UninstallWorker::UninstallWorker(UninstallEngine* engine, const QList<ApplicationInfo>& appList)
    : m_engine(engine)
    , m_appList(appList)
{
}

void UninstallWorker::doWork() {
    for (int i = 0; i < m_appList.size(); ++i) {
        if (m_engine->m_shouldStop) {
            break;
        }
        
        const ApplicationInfo& appInfo = m_appList[i];
        m_engine->m_currentIndex = i;
        
        emit uninstallStarted(appInfo.name);
        LOG_INFO(QString("开始卸载应用: %1").arg(appInfo.name));
        
        UninstallResult result = UninstallResult::Success;
        
        try {
            // 安全检查
            SafetyChecker& safety = SafetyChecker::instance();
            if (appInfo.isSystemApp || safety.isSystemApplication(appInfo.name, appInfo.publisher)) {
                LOG_WARNING(QString("跳过系统应用: %1").arg(appInfo.name));
                emit uninstallError(appInfo.name, "这是系统关键应用，无法卸载");
                result = UninstallResult::Failed;
            } else {
                // 创建备份（如果启用）
                if (m_engine->m_createBackup) {
                    m_engine->createBackup(appInfo);
                }
                
                // 1. 尝试运行原生卸载程序
                bool nativeSuccess = m_engine->runNativeUninstaller(appInfo);
                
                // 2. 执行深度清理
                bool deepCleanSuccess = m_engine->performDeepClean(appInfo);
                
                if (nativeSuccess && deepCleanSuccess) {
                    result = UninstallResult::Success;
                } else if (nativeSuccess || deepCleanSuccess) {
                    result = UninstallResult::PartialSuccess;
                } else {
                    result = UninstallResult::Failed;
                }
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR(QString("卸载过程中发生错误: %1").arg(e.what()));
            emit uninstallError(appInfo.name, QString("卸载失败: %1").arg(e.what()));
            result = UninstallResult::Failed;
        } catch (...) {
            LOG_ERROR("卸载过程中发生未知错误");
            emit uninstallError(appInfo.name, "卸载失败: 未知错误");
            result = UninstallResult::Failed;
        }
        
        emit uninstallFinished(appInfo.name, result);
        LOG_INFO(QString("应用 %1 卸载完成，结果: %2").arg(appInfo.name).arg(static_cast<int>(result)));
    }
    
    emit finished();
}

QString UninstallEngine::createBackup(const ApplicationInfo& appInfo) {
    // 备份功能的基本实现
    QString backupPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BTU_Backups";
    QDir().mkpath(backupPath);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString backupDir = QString("%1/%2_%3").arg(backupPath, appInfo.name, timestamp);
    
    LOG_INFO(QString("创建备份到: %1").arg(backupDir));
    
    // 这里可以实现具体的备份逻辑
    // 例如压缩安装目录、导出注册表项等
    
    return backupDir;
}

#include "UninstallEngine.moc"
