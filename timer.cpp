#include "timer.hpp"

#include <cmath>

std::optional<float> timer::PeriodicTimer::tick(float dt) {
    accumulated += dt;
    if (accumulated >= interval) {
        auto result{accumulated};
        accumulated = std::fmod(accumulated, interval);
        return result;
    }
    return {};
}
