#pragma once

#include <chrono>
#include <cmath>
#include <iostream>

#include "logging.hpp"

struct Signal {
    float value = 0.0f;
    float inv_tau = 1.0f;

    inline void update(float sample, float dt) {
        float alpha = std::min(dt * inv_tau, 1.0f);
        value += alpha * (sample - value);
    }
};

struct Profiler {
    using clock = std::chrono::steady_clock;
    const char* name;
    float interval = 1.0f;
    Signal signal{};
    clock::time_point start = clock::now();
    clock::time_point last_log = clock::now();

    inline void begin() { start = clock::now(); }

    inline void end(float dt) {
        auto now = clock::now();
        float sample
            = std::chrono::duration<float, std::micro>(now - start).count();
        signal.update(sample, dt);
        if (std::chrono::duration<float>(now - last_log).count() > interval) {
            logging::debug("avg time on {}: {:.2f} us", name, signal.value);
            last_log = now;
        }
    }
};

struct ScopedProfiler {
    Profiler& profiler;
    float dt;

    ScopedProfiler(Profiler& p, float dt) : profiler(p), dt(dt) { profiler.begin(); }

    ~ScopedProfiler() { profiler.end(dt); }
};

struct SignalMonitor {
    const char* msg;
    float low;
    float high;
    Signal signal{};
    bool active = false;

    inline void update(float sample, float dt) {
        signal.update(sample, dt);
        float v = signal.value;
        if (!active && v > high) {
            active = true;
            logging::warn("{}", msg);
        }
        if (active && v < low) {
            active = false;
        }
    }
};