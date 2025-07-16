#pragma once

#include "AppScanner.h"
#include <QString>
#include <QStringList>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

enum class UninstallResult {
    Success,
    Failed,
    Cancelled,
    PartialSuccess
};

struct UninstallProgress {
    QString currentOperation;
    int filesProcessed;
    int totalFiles;
    int registryKeysProcessed;
    int totalRegistryKeys;
    qint64 bytesFreed;
    bool isComplete;
    
    UninstallProgress() : filesProcessed(0), totalFiles(0), 
                         registryKeysProcessed(0), totalRegistryKeys(0),
                         bytesFreed(0), isComplete(false) {}
};

class UninstallEngine : public QObject {
    Q_OBJECT

public:
    explicit UninstallEngine(QObject* parent = nullptr);
    ~UninstallEngine();
    
    // 卸载单个应用
    void uninstallApplication(const ApplicationInfo& appInfo);
    
    // 批量卸载应用
    void uninstallApplications(const QList<ApplicationInfo>& appList);
    
    // 停止卸载过程
    void stopUninstall();
    
    // 检查是否正在卸载
    bool isUninstalling() const;
    
    // 设置是否创建备份
    void setCreateBackup(bool enabled);
    
    // 设置是否强制删除
    void setForceDelete(bool enabled);

signals:
    void uninstallStarted(const QString& appName);
    void uninstallFinished(const QString& appName, UninstallResult result);
    void uninstallProgress(const QString& appName, const UninstallProgress& progress);
    void uninstallError(const QString& appName, const QString& error);
    void allUninstallsFinished();

private slots:
    void onUninstallFinished();

private:
    void performUninstall(const ApplicationInfo& appInfo);
    bool runNativeUninstaller(const ApplicationInfo& appInfo);
    bool performDeepClean(const ApplicationInfo& appInfo);
    bool deleteDirectory(const QString& dirPath);
    bool deleteRegistryKeys(const ApplicationInfo& appInfo);
    bool cleanUserData(const ApplicationInfo& appInfo);
    bool cleanTemporaryFiles(const ApplicationInfo& appInfo);
    bool cleanStartupEntries(const ApplicationInfo& appInfo);
    bool cleanServices(const ApplicationInfo& appInfo);
    
    QStringList findRelatedFiles(const QString& appName);
    QStringList findRelatedRegistryKeys(const QString& appName);
    QString createBackup(const ApplicationInfo& appInfo);
    
    QThread* m_uninstallThread;
    QMutex m_mutex;
    bool m_isUninstalling;
    bool m_shouldStop;
    bool m_createBackup;
    bool m_forceDelete;
    QList<ApplicationInfo> m_uninstallQueue;
    int m_currentIndex;
};

class UninstallWorker : public QObject {
    Q_OBJECT

public:
    explicit UninstallWorker(UninstallEngine* engine, const QList<ApplicationInfo>& appList);
    
public slots:
    void doWork();
    
signals:
    void finished();
    void uninstallStarted(const QString& appName);
    void uninstallFinished(const QString& appName, UninstallResult result);
    void uninstallProgress(const QString& appName, const UninstallProgress& progress);
    void uninstallError(const QString& appName, const QString& error);
    
private:
    UninstallEngine* m_engine;
    QList<ApplicationInfo> m_appList;
};
