#pragma once

#include "signal.hpp"

constexpr float FRACTION_WARN = 50.0f;
constexpr float FRACTION_ERROR = 90.0f;

// Live performance diagnostics for the tracker itself.
struct Diagnostics {
    // Tracker's own render loop frame time.
    Signal frame_time{"frame time", "seconds"};
    // Time spent update_slow (called roughly every 0.1s).
    Profiler slow_update{{"slow update time", "seconds"}};
    // Time spent in update_fast (called every frame).
    Profiler fast_update{{"fast update time", "seconds"}};
    // Percentage of update_slow calls that failed
    // (e.g. a bad memory read, often transient during level loads).
    Signal slow_update_error_rate{
        "slow update failure rate", "%", FRACTION_WARN, FRACTION_ERROR
    };
    // Percentage of update_fast calls that failed.
    Signal fast_update_error_rate{
        "fast update failure rate", "%", FRACTION_WARN, FRACTION_ERROR
    };
    Signal bytes_read_rate{"bytes read", "B/s"};
    Signal bytes_written_rate{"bytes written", "B/s"};
};
