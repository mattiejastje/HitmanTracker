#include "spdlog.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "logging.hpp"

spdlog::level::level_enum spdlog_level[7]{
    spdlog::level::off,
    spdlog::level::trace,
    spdlog::level::err,
    spdlog::level::warn,
    spdlog::level::info,
    spdlog::level::debug,
    spdlog::level::trace
};

class SpdlogLogger : public logging::Logger {
    virtual void log(int level, const std::string& msg) {
        assert((level >= 0) && (level < 7));
        spdlog::log(spdlog_level[level], msg);
    };
};

void spdlog_init() {
    auto logger = spdlog::stdout_color_mt("HitmanTracker");
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%t] [%^%l%$] %v");
    spdlog::set_default_logger(logger);
    logging::logger = std::make_shared<SpdlogLogger>();
}

void spdlog_set_level(int log_level, int log_flush_level) {
    spdlog::set_level(spdlog_level[log_level]);
    spdlog::flush_on(spdlog_level[log_flush_level]);
}
