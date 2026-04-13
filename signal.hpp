#pragma once

#include <chrono>
#include <optional>

struct Signal {
    const char* name;
    const char* unit;
    std::optional<float> high = std::nullopt;
    float value = 0.0f;
    float inv_tau = 1.0f;
    float interval = 1.0f;
    float now = 0.0f;

    void update(float sample, float dt);
};

struct Profiler {
    using clock = std::chrono::steady_clock;
    Signal signal{};
    clock::time_point start = clock::now();

    void begin();
    void end(float dt);
};

struct ScopedProfiler {
    Profiler& profiler;
    float dt;

    ScopedProfiler(Profiler& p, float dt);

    ~ScopedProfiler();
};
