#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <optional>

#include "logging.hpp"

struct Signal {
    const char* name;
    const char* unit;
    std::optional<float> high = std::nullopt;
    float value = 0.0f;
    float inv_tau = 1.0f;
    float interval = 1.0f;
    float now = 0.0f;

    inline void update(float sample, float dt) {
        float alpha = std::min(dt * inv_tau, 1.0f);
        value += alpha * (sample - value);
        now += dt;
        if (now > interval) {
            if (high && (value > *high)) {
                logging::error("{} = {:g} {} (too high)", name, value, unit);
            } else {
                logging::debug("{} = {:g} {}", name, value, unit);
            }
            now -= interval;
        }
    }
};

struct Profiler {
    using clock = std::chrono::steady_clock;
    Signal signal{};
    clock::time_point start = clock::now();

    inline void begin() { start = clock::now(); }

    inline void end(float dt) {
        auto now = clock::now();
        float sample = std::chrono::duration<float>(now - start).count();
        signal.update(sample, dt);
    }
};

struct ScopedProfiler {
    Profiler& profiler;
    float dt;

    ScopedProfiler(Profiler& p, float dt) : profiler(p), dt(dt) {
        profiler.begin();
    }

    ~ScopedProfiler() { profiler.end(dt); }
};
