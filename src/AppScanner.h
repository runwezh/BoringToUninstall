#pragma once

#include <QString>
#include <QStringList>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QDateTime>

struct ApplicationInfo {
    QString name;
    QString displayName;
    QString version;
    QString publisher;
    QString installDate;
    QString installLocation;
    QString uninstallString;
    QString estimatedSize;
    QString registryKey;
    bool isSystemApp;
    bool canUninstall;
    
    ApplicationInfo() : isSystemApp(false), canUninstall(true) {}
};

class AppScanner : public QObject {
    Q_OBJECT

public:
    explicit AppScanner(QObject* parent = nullptr);
    ~AppScanner();
    
    // 开始扫描已安装应用
    void startScan();
    
    // 停止扫描
    void stopScan();
    
    // 获取应用列表
    QList<ApplicationInfo> getApplications() const;
    
    // 根据名称搜索应用
    QList<ApplicationInfo> searchApplications(const QString& keyword) const;
    
    // 刷新应用列表
    void refreshApplications();
    
    // 检查是否正在扫描
    bool isScanning() const;

signals:
    void scanStarted();
    void scanFinished();
    void applicationFound(const ApplicationInfo& appInfo);
    void scanProgress(int current, int total);
    void scanError(const QString& error);

private slots:
    void onScanFinished();

private:
    void scanRegistry();
    void scanRegistryKey(const QString& keyPath);
    ApplicationInfo parseRegistryEntry(const QString& keyPath, const QString& subKey);
    QString formatSize(qint64 bytes) const;
    QString parseInstallDate(const QString& dateStr) const;
    
    QThread* m_scanThread;
    QMutex m_mutex;
    QList<ApplicationInfo> m_applications;
    bool m_isScanning;
    bool m_shouldStop;
};

class ScanWorker : public QObject {
    Q_OBJECT

public:
    explicit ScanWorker(AppScanner* scanner);
    
public slots:
    void doWork();
    
signals:
    void finished();
    void applicationFound(const ApplicationInfo& appInfo);
    void progress(int current, int total);
    void error(const QString& error);
    
private:
    AppScanner* m_scanner;
};
