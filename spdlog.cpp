#include "spdlog.hpp"

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

constexpr auto LOG_PATTERN = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%&] %v";
constexpr auto LOG_TIME_ONLY_PATTERN = "%H:%M:%S";

static std::shared_ptr<CounterSink> g_counter_sink = nullptr;
static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> g_file_sink
    = nullptr;
static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> g_ring_sink = nullptr;

static auto make_file_sink() {
    return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/HitmanTracker.log", 1024 * 1024 * 10, 5
    );
};

void spdlog_init() {
    std::vector<spdlog::sink_ptr> sinks;
#ifndef NDEBUG
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
    g_file_sink = make_file_sink();
    sinks.push_back(g_file_sink);
    g_ring_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(100);
    g_ring_sink->set_level(spdlog::level::err);
    sinks.push_back(g_ring_sink);
    g_counter_sink = std::make_shared<CounterSink>();
    g_counter_sink->set_level(spdlog::level::err);
    sinks.push_back(g_counter_sink);
    auto logger = std::make_shared<spdlog::logger>(
        "HitmanTracker", sinks.begin(), sinks.end()
    );
    logger->set_pattern(LOG_PATTERN);
    spdlog::set_default_logger(logger);
}

void spdlog_set_level(bool capture_trace) {
    auto level = capture_trace ? spdlog::level::trace : spdlog::level::debug;
    spdlog::set_level(level);
    spdlog::flush_on(level);
}

std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> spdlog_ring_sink() {
    return g_ring_sink;
}

std::shared_ptr<CounterSink> spdlog_counter_sink() { return g_counter_sink; }

std::filesystem::path spdlog_log_dir() {
    return std::filesystem::absolute("logs");
}

bool spdlog_clear_log_files() {
    if (!g_file_sink) return false;
    auto logger = spdlog::default_logger();
    logger->flush();
    auto& sinks = logger->sinks();
    std::erase(
        sinks, std::static_pointer_cast<spdlog::sinks::sink>(g_file_sink)
    );
    g_file_sink.reset();
    bool ok = true;
    std::error_code ec;
    for (auto& entry :
         std::filesystem::directory_iterator(spdlog_log_dir(), ec)) {
        if (entry.is_regular_file(ec) && entry.path().extension() == ".log") {
            if (!std::filesystem::remove(entry.path(), ec)) ok = false;
        }
    }
    g_file_sink = make_file_sink();
    sinks.push_back(g_file_sink);
    spdlog::info("Log files cleared");
    spdlog::info("Hitman Tracker v" APP_VERSION);
    return ok;
}

std::string spdlog_format_entry(const spdlog::details::log_msg_buffer& msg) {
    static spdlog::pattern_formatter formatter{LOG_PATTERN};
    spdlog::memory_buf_t dest;
    formatter.format(msg, dest);
    return fmt::to_string(dest);
}

std::string spdlog_format_time(const spdlog::details::log_msg_buffer& msg) {
    static spdlog::pattern_formatter formatter{LOG_TIME_ONLY_PATTERN};
    spdlog::memory_buf_t dest;
    formatter.format(msg, dest);
    return fmt::to_string(dest);
}