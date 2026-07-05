#pragma once

namespace timer {

struct PeriodicTimer {
    const float interval;
    float accumulated = 0;

    bool tick(float dt);
};

}  // namespace timer
