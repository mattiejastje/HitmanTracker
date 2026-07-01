#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "../stats.hpp"

namespace hitman_common {

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

struct Stats {
    float time;
    int map;
    MapStage map_stage;
    int32_t difficulty;
    StatsValue<std::string> rating;
    StatsValue<int32_t> shots_fired;
    StatsValue<int32_t> headshots;
    StatsValue<int32_t> enemies_killed;
    StatsValue<int32_t> enemies_wounded;
    StatsValue<int32_t> innocents_killed;
    StatsValue<int32_t> innocents_wounded;
    StatsValue<int32_t> close_encounters;
    StatsValue<int32_t> alerts;
    StatsValue<std::string> stealth;
    StatsValue<std::string> aggression;
};

void process_game_stats(
    StatsFunc measure_aggression,
    StatsFunc measure_stealth,
    const StatsArray<int32_t> game_stats,
    Stats& stats
);

}  // namespace hitman_common