#include "format_duration.hpp"

#include <fmt/format.h>

#include <chrono>

std::string format_duration(float seconds) {
    std::chrono::duration<float> duration(seconds);
    auto mins = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= mins;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= secs;
    auto millisecs
        = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return fmt::format(
        "{}:{:02}.{:03}", mins.count(), secs.count(), millisecs.count()
    );
}
