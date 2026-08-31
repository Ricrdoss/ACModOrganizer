#include "Logger.hpp"

namespace acbo {

Logger& Logger::instance() {
    static Logger s_instance;
    return s_instance;
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Success: return "SUCCESS";
    }
    return "INFO";
}

std::string Logger::getTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto timeT = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm bt{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&bt, &timeT);
#else
    localtime_r(&timeT, &bt);
#endif

    std::ostringstream ss;
    ss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::log(LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const std::string ts = getTimestamp();
    const std::string lvlStr = levelToString(level);

    std::string prefix;
    switch (level) {
        case LogLevel::Debug:   prefix = "\033[36m[DEBUG]\033[0m "; break;
        case LogLevel::Info:    prefix = "\033[34m[INFO]\033[0m "; break;
        case LogLevel::Warning: prefix = "\033[33m[WARN]\033[0m "; break;
        case LogLevel::Error:   prefix = "\033[31m[ERROR]\033[0m "; break;
        case LogLevel::Success: prefix = "\033[32m[SUCCESS]\033[0m "; break;
    }

    std::cout << "[" << ts << "] " << prefix << message << std::endl;

    emit logMessageEmitted(
        QString::fromStdString(lvlStr),
        QString::fromUtf8(message.data(), static_cast<qsizetype>(message.size())),
        QString::fromStdString(ts)
    );
}

void Logger::debug(std::string_view message)   { log(LogLevel::Debug, message); }
void Logger::info(std::string_view message)    { log(LogLevel::Info, message); }
void Logger::warn(std::string_view message)    { log(LogLevel::Warning, message); }
void Logger::error(std::string_view message)   { log(LogLevel::Error, message); }
void Logger::success(std::string_view message) { log(LogLevel::Success, message); }

void Logger::logQt(LogLevel level, const QString& message) {
    const std::string utf8 = message.toStdString();
    log(level, utf8);
}

} // namespace acbo
