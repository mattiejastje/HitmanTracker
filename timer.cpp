#include "timer.hpp"

bool timer::PeriodicTimer::tick(float dt) {
    accumulated += dt;
    if (accumulated >= interval) {
        accumulated -= interval;
        return true;
    }
    return false;
}
