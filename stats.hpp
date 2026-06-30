#pragma once

#include <cstdint>
#include <optional>
#include <string>

enum class Status { GREEN, YELLOW, RED };

template <class T>
struct StatsValue {
    T value;
    std::optional<Status> status;
};

StatsValue<int32_t> stats_value(int32_t value, bool required = true);

enum class MapStage { pre, main, post };
