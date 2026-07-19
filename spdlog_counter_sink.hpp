#pragma once

#include <spdlog/spdlog.h>

#include <atomic>

class CounterSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    std::atomic<int> count{0};

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        ++count;
    }

    void flush_() override {}
};
