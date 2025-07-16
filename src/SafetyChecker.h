#pragma once

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

class SafetyChecker {
public:
    static SafetyChecker& instance();
    
    // 检查路径是否安全可删除
    bool isSafeToDelete(const QString& path);
    
    // 检查是否为系统关键目录
    bool isSystemCriticalPath(const QString& path);
    
    // 检查是否为Windows系统文件
    bool isWindowsSystemFile(const QString& filePath);
    
    // 检查是否为重要的系统应用
    bool isSystemApplication(const QString& appName, const QString& publisher);
    
    // 获取受保护的路径列表
    QStringList getProtectedPaths() const;
    
    // 获取受保护的应用程序列表
    QStringList getProtectedApplications() const;
    
    // 验证注册表键是否安全删除
    bool isSafeRegistryKey(const QString& keyPath);

private:
    SafetyChecker();
    void initializeProtectedPaths();
    void initializeProtectedApplications();
    void initializeProtectedRegistryKeys();
    
    QStringList m_protectedPaths;
    QStringList m_protectedApplications;
    QStringList m_protectedRegistryKeys;
    QStringList m_systemPublishers;
};
