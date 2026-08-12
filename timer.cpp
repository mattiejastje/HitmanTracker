#include "timer.hpp"

#include <cmath>

bool timer::PeriodicTimer::tick(float dt) {
    accumulated += dt;
    if (accumulated >= interval) {
        accumulated = std::fmod(accumulated, interval);
        return true;
    }
    return false;
}
