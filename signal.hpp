#pragma once

#include <chrono>
#include <optional>

// Raw signal.
struct RawSignal {
    const char* name;
    const char* unit;
    std::optional<float> warn = std::nullopt;
    std::optional<float> err = std::nullopt;
    float value = 0.0f;
};

// Signal with smooth updates.
struct Signal : public RawSignal {
    float inv_tau = 1.0f;
    float interval = 1.0f;
    float now = 0.0f;

    void update(float sample, float dt);
};

// Profiler signal: measures smoothed time between begin and end.
struct Profiler {
    using clock = std::chrono::steady_clock;
    Signal signal{};
    clock::time_point start = clock::now();

    void begin();
    void end(float dt);
};

// Profiler for a scope.
struct ScopedProfiler {
    Profiler& profiler;
    float dt;

    ScopedProfiler(Profiler& p, float dt);

    ~ScopedProfiler();
};
