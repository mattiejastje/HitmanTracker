#include "stats.hpp"

static Status status(int32_t value, bool required) {
    return required ? (value ? Status::RED : Status::YELLOW) : Status::GREEN;
}

StatsValue<int32_t> stats_value(int32_t value, bool required) {
    return StatsValue{value, status(value, required)};
}
