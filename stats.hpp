#pragma once

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>

enum class Status { GREEN, YELLOW, RED };

template <class T>
struct StatsValue {
    T value;
    std::optional<Status> status;
};

StatsValue<int32_t> stats_value(int32_t value, bool required = true);

StatsValue<int32_t> merge_stats_values(
    std::initializer_list<StatsValue<int32_t>> values
);

enum class MapStage { pre, main, post };
