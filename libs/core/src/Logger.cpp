#include "ontoflow/core/Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>

namespace of::core {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : m_logLevel(LogLevel::Info | LogLevel::Debug | LogLevel::Warn | LogLevel::Error) {
}

Logger::~Logger() {
    if (m_fout.is_open()) {
        m_fout.close();
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_writeMutex);
    m_logLevel = level;
}

LogLevel Logger::getLogLevel() const {
    return m_logLevel;
}

void Logger::setLogFile(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(m_writeMutex);
    if (m_fout.is_open()) {
        m_fout.close();
    }

    if (!logFilePath.empty()) {
        m_fout.open(logFilePath, std::ios::out | std::ios::app);
        if (!m_fout.is_open()) {
            std::cerr << "[Error] Failed to open log file: " << logFilePath << std::endl;
        }
    }
}

void Logger::write(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_writeMutex);
    if (m_fout.is_open()) {
        m_fout << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

// -----------------------------------------------------------------------------

LogMessage::LogMessage(LogLevel level, const char* levelName) {
    Logger& logger = Logger::getInstance();
    m_enabled = (logger.getLogLevel() & level) != LogLevel::None;

    if (m_enabled) {
        // Get current time
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_buf, &in_time_t);
#else
        localtime_r(&in_time_t, &tm_buf);
#endif

        m_buffer << "[" << std::put_time(&tm_buf, "%Y-%m-%d %X") << "] "
                 << "[" << levelName << "] ";
    }
}

LogMessage::~LogMessage() {
    if (m_enabled) {
        Logger::getInstance().write(m_buffer.str());
    }
}

LogMessage& LogMessage::operator<<(std::ostream& (*manip)(std::ostream&)) {
    if (m_enabled) {
        manip(m_buffer);
    }
    return *this;
}

}  // namespace of::core
