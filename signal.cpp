#include "signal.hpp"

#include <spdlog/spdlog.h>

#include <cmath>

void Signal::update(float sample, float dt) {
    float alpha = std::min(dt * inv_tau, 1.0f);
    value += alpha * (sample - value);
    now += dt;
    if (now > interval) {
        if (high && (value > *high)) {
            spdlog::error("{} = {:g} {} (too high)", name, value, unit);
        } else {
            spdlog::debug("{} = {:g} {}", name, value, unit);
        }
        now = std::fmod(now, interval);
    }
}

void Profiler::begin() { start = clock::now(); }

void Profiler::end(float dt) {
    auto now = clock::now();
    float sample = std::chrono::duration<float>(now - start).count();
    signal.update(sample, dt);
}

ScopedProfiler::ScopedProfiler(Profiler& p, float dt) : profiler(p), dt(dt) {
    profiler.begin();
}

ScopedProfiler::~ScopedProfiler() { profiler.end(dt); }