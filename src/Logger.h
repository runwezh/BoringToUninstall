#pragma once

#include <QString>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& instance();
    void log(LogLevel level, const QString& message);
    void setLogFile(const QString& filename);
    QString getLogContent() const;
    void clearLog();

private:
    Logger();
    ~Logger();
    
    QString formatMessage(LogLevel level, const QString& message) const;
    QString levelToString(LogLevel level) const;
    
    QFile* m_logFile;
    QString m_logPath;
    QTextStream* m_logStream;
};

// 便利宏
#define LOG_DEBUG(msg) Logger::instance().log(LogLevel::Debug, msg)
#define LOG_INFO(msg) Logger::instance().log(LogLevel::Info, msg)
#define LOG_WARNING(msg) Logger::instance().log(LogLevel::Warning, msg)
#define LOG_ERROR(msg) Logger::instance().log(LogLevel::Error, msg)
