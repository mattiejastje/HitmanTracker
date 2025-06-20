#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "../stats.hpp"

// these stats are common to Hitman 2 SA and Hitman Contracts
struct CommonGameStats {
    int32_t headshots;          // 0x208
    int32_t enemies_wounded;    // 0x20C
    int32_t enemies_killed;     // 0x210
    int32_t innocents_wounded;  // 0x214
    int32_t innocents_killed;   // 0x218
    int32_t alerts;             // 0x21C
    int32_t close_encounters;   // 0x220
};

static_assert(sizeof(CommonGameStats) == 28);

using StatsArray = std::array<int32_t, 8>;

void process_common_game_stats(
    const std::vector<StatsArray>& silent_assassin_combinations,
    const CommonGameStats& game_stats,
    Stats& stats
);

std::string get_simple_rating_value(Status status);
