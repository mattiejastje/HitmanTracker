#pragma once

#include <format>
#include <string>

namespace logging {

// subclass this to define your logger
class Logger {
public:
    virtual void log(int level, const std::string& msg) = 0;
    virtual bool should_log(int level) = 0;
    virtual ~Logger() {};
};

// assign this to activate your logger
extern std::shared_ptr<Logger> logger;

template <typename... Args>
void critical(std::format_string<Args...> fmt, Args&&... args) {
    if (logger && logger->should_log(1))
        logger->log(1, std::format(fmt, std::forward<Args>(args)...));
};

template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args) {
    if (logger && logger->should_log(2))
        logger->log(2, std::format(fmt, std::forward<Args>(args)...));
};

template <typename... Args>
void warn(std::format_string<Args...> fmt, Args&&... args) {
    if (logger && logger->should_log(3))
        logger->log(3, std::format(fmt, std::forward<Args>(args)...));
};

template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args) {
    if (logger && logger->should_log(4))
        logger->log(4, std::format(fmt, std::forward<Args>(args)...));
};

template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args) {
    if (logger && logger->should_log(5))
        logger->log(5, std::format(fmt, std::forward<Args>(args)...));
};

template <typename... Args>
void trace(std::format_string<Args...> fmt, Args&&... args) {
    if (logger && logger->should_log(6))
        logger->log(6, std::format(fmt, std::forward<Args>(args)...));
};

}  // namespace logging
