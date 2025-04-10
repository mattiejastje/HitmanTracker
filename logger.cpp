#include "logger.hpp"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

void logger_init() {
    g_logger = spdlog::rotating_logger_mt(
        "HitmanTracker", "HitmanTracker.log", 10000000, 5
    );
    g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%t] [%^%l%$] %v");
}
