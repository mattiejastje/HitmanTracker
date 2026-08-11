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

StatsValue<int32_t> merge_stats_values(
    std::initializer_list<StatsValue<int32_t>> values
) {
    int32_t sum = 0;
    std::optional<Status> worst;
    for (const auto& value : values) {
        sum += value.value;
        if (value.status && (!worst || *value.status > *worst)) {
            worst = value.status;
        }
    }
    return StatsValue<int32_t>{sum, worst};
}