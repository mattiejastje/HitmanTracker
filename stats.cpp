#include "stats.hpp"

// GREEN  = stat is not tracked
// YELLOW = stat is tracked and currently clean (0); borderline, since
//          it could still become non-zero before the checkpoint ends
// RED    = stat is tracked and already non-zero (violated)
static Status status(int32_t value, bool is_tracked) {
    return is_tracked ? (value ? Status::RED : Status::YELLOW) : Status::GREEN;
}

StatsValue<int32_t> stats_value(int32_t value, bool required) {
    return StatsValue{value, status(value, required)};
}
