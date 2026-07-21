#pragma once

#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <string>

void spdlog_init();
void spdlog_set_level(bool capture_trace);
std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> spdlog_ring_sink();
std::filesystem::path spdlog_log_dir();
std::string spdlog_format_entry(const spdlog::details::log_msg_buffer& msg);
std::string spdlog_format_time(const spdlog::details::log_msg_buffer& msg);
