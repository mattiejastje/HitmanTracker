#include "logger.hpp"

#include <spdlog/sinks/rotating_file_sink.h>

void logger_init() {
    auto logger = spdlog::rotating_logger_mt(
        "HitmanTracker", "HitmanTracker.log", 10000000, 5
    );
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%t] [%^%l%$] %v");
    spdlog::set_default_logger(logger);
}
