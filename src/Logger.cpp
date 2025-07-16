#include "Logger.h"
#include <QApplication>
#include <QDebug>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : m_logFile(nullptr), m_logStream(nullptr) {
    // 创建日志目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    
    // 设置默认日志文件
    m_logPath = appDataPath + "/btu.log";
    setLogFile(m_logPath);
}

Logger::~Logger() {
    if (m_logStream) {
        delete m_logStream;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

void Logger::setLogFile(const QString& filename) {
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
    
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    
    m_logFile = new QFile(filename);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setEncoding(QStringConverter::Utf8);
    }
}

void Logger::log(LogLevel level, const QString& message) {
    QString formattedMessage = formatMessage(level, message);
    
    // 输出到控制台
    qDebug() << formattedMessage;
    
    // 写入日志文件
    if (m_logStream) {
        *m_logStream << formattedMessage << "\n";
        m_logStream->flush();
    }
}

QString Logger::formatMessage(LogLevel level, const QString& message) const {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString levelStr = levelToString(level);
    return QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
}

QString Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

QString Logger::getLogContent() const {
    if (!m_logFile) return QString();
    
    QFile file(m_logPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        return in.readAll();
    }
    return QString();
}

void Logger::clearLog() {
    if (m_logFile) {
        m_logFile->close();
        m_logFile->remove();
        
        // 重新打开日志文件
        if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            if (m_logStream) {
                m_logStream->setDevice(m_logFile);
            }
        }
    }
}
