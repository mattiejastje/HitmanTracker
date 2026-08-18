#pragma once

#include <optional>

namespace timer {

struct PeriodicTimer {
    const float interval;
    float accumulated = 0;

    std::optional<float> tick(float dt);
};

}  // namespace timer
