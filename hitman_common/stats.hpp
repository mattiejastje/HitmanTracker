#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "../stats.hpp"

inline constexpr std::size_t SHOTS_FIRED = 0;
inline constexpr std::size_t CLOSE_ENCOUNTERS = 1;
inline constexpr std::size_t HEADSHOTS = 2;
inline constexpr std::size_t ALERTS = 3;
inline constexpr std::size_t ENEMIES_KILLED = 4;
inline constexpr std::size_t ENEMIES_WOUNDED = 5;
inline constexpr std::size_t INNOCENTS_KILLED = 6;
inline constexpr std::size_t INNOCENTS_WOUNDED = 7;

template <typename T>
using StatsArray = std::array<T, 8>;

using StatsFunc = std::function<int32_t(const StatsArray<int32_t>&)>;

void process_common_game_stats(
    StatsFunc measure_aggression,
    StatsFunc measure_stealth,
    const StatsArray<int32_t> game_stats,
    Stats& stats
);