#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <QObject>
#include <QString>

namespace acbo {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Success
};

class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& instance();

    void log(LogLevel level, std::string_view message);
    void debug(std::string_view message);
    void info(std::string_view message);
    void warn(std::string_view message);
    void error(std::string_view message);
    void success(std::string_view message);

    void logQt(LogLevel level, const QString& message);

signals:
    void logMessageEmitted(const QString& levelStr, const QString& message, const QString& timestamp);

private:
    Logger() = default;
    ~Logger() override = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex m_mutex;
    static std::string levelToString(LogLevel level);
    static std::string getTimestamp();
};

#define LOG_DEBUG(msg)   ::acbo::Logger::instance().debug(msg)
#define LOG_INFO(msg)    ::acbo::Logger::instance().info(msg)
#define LOG_WARN(msg)    ::acbo::Logger::instance().warn(msg)
#define LOG_ERROR(msg)   ::acbo::Logger::instance().error(msg)
#define LOG_SUCCESS(msg) ::acbo::Logger::instance().success(msg)

} // namespace acbo
