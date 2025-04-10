#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> g_logger;

#define LOG \
    if (g_logger) g_logger

void logger_init();
