#include <chrono>
#include <cmath>
#include <iostream>

#include "logging.hpp"

// simple profiler with exponential smoothing
struct Profiler {
    using clock = std::chrono::steady_clock;

    const char* name;
    double inv_tau = 1.0;
    double interval = 1.0;
    double avg = 0.0;
    clock::time_point start = clock::now();
    clock::time_point last_update = clock::now();
    clock::time_point last_log = clock::now();
    bool initialized = false;

    void begin() { start = clock::now(); }

    void end() {
        auto now = clock::now();
        double sample
            = std::chrono::duration<double, std::micro>(now - start).count();
        double dt = std::chrono::duration<double>(now - last_update).count();
        double alpha = 1.0 - std::exp(-dt * inv_tau);
        if (!initialized) {
            avg = sample;
            initialized = true;
        } else {
            avg = (1.0 - alpha) * avg + alpha * sample;
        }
        last_update = now;
        if (std::chrono::duration<double>(now - last_log).count() > interval) {
            logging::debug("avg time spent on {}: {}", name, avg);
            last_log = now;
        }
    }
};

struct ScopedProfiler {
    Profiler& profiler;

    ScopedProfiler(Profiler& p) : profiler(p) { profiler.begin(); }

    ~ScopedProfiler() { profiler.end(); }
};