#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace of::core {

/**
 * @brief Log levels for the application.
 * Used as a bitmask.
 */
enum class LogLevel : uint16_t {
    None = 0x00,
    Info = 0x01,
    Debug = 0x02,
    Warn = 0x04,
    Error = 0x08,
    All = 0xFF
};

// Overload bitwise operators for convenient masking
inline LogLevel operator|(LogLevel lhs, LogLevel rhs) {
    return static_cast<LogLevel>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}

inline LogLevel operator&(LogLevel lhs, LogLevel rhs) {
    return static_cast<LogLevel>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
}

inline LogLevel& operator|=(LogLevel& lhs, LogLevel rhs) {
    lhs = lhs | rhs;
    return lhs;
}

/**
 * @brief Singleton class managing logging configuration and output stream.
 */
class Logger {
   public:
    /**
     * @brief Access the singleton instance.
     * @return Reference to the Logger instance.
     */
    static Logger& getInstance();

    /**
     * @brief Set the active log levels.
     * @param level Bitmask of LogLevel values.
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Get the current active log levels.
     * @return Current LogLevel bitmask.
     */
    LogLevel getLogLevel() const;

    /**
     * @brief Redirect log output to a file.
     * @param logFilePath Path to the log file.
     */
    void setLogFile(const std::string& logFilePath);

    /**
     * @brief Thread-safe writing to the output stream.
     * @param message The fully formatted message to write.
     */
    void write(const std::string& message);

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

   private:
    Logger();
    ~Logger();

    LogLevel m_logLevel;
    std::ofstream m_fout;
    std::mutex m_writeMutex;
};

/**
 * @brief Temporary object to construct a log message.
 * Buffers output and writes to the Logger singleton on destruction.
 */
class LogMessage {
   public:
    LogMessage(LogLevel level, const char* levelName);
    ~LogMessage();

    template <typename T>
    LogMessage& operator<<(const T& msg) {
        if (m_enabled) {
            m_buffer << msg;
        }
        return *this;
    }

    // Handle manipulators like std::endl
    LogMessage& operator<<(std::ostream& (*manip)(std::ostream&));

   private:
    bool m_enabled;
    std::ostringstream m_buffer;
};

}  // namespace of::core

/**
 * @brief Macro to start logging.
 * Usage: LOG(Info) << "Message";
 */
#define LOG(Level) of::core::LogMessage(of::core::LogLevel::Level, #Level)
