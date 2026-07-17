#include "spdlog.hpp"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

spdlog::level::level_enum spdlog_level[7]{
    spdlog::level::off,
    spdlog::level::critical,
    spdlog::level::err,
    spdlog::level::warn,
    spdlog::level::info,
    spdlog::level::debug,
    spdlog::level::trace
};

void spdlog_init() {
    std::vector<spdlog::sink_ptr> sinks;
#ifndef NDEBUG
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
    sinks.push_back(
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "HitmanTracker.log", 1024 * 1024 * 10, 5
        )
    );
    auto logger = std::make_shared<spdlog::logger>(
        "HitmanTracker", sinks.begin(), sinks.end()
    );
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%t] [%^%l%$] [%&] %v");
    spdlog::set_default_logger(logger);
}

void spdlog_set_level(int log_level, int log_flush_level) {
    spdlog::set_level(spdlog_level[log_level]);
    spdlog::flush_on(spdlog_level[log_flush_level]);
}