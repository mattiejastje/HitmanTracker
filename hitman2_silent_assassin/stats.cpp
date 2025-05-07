#include "stats.hpp"

#include <unordered_map>
#include <vector>

#include "../mem/read_write.hpp"

// unordered_map for fast lookup
const std::unordered_map<std::string, int> scenes = {
    {R"(SCENES\C0-1\C0-1__MAIN.gms)", 1},
    {R"(SCENES\C1-1\C1-1__MAIN.gms)", 2},
    {R"(SCENES\C2-1\C2-1__MAIN.gms)", 3},
    {R"(SCENES\C2-2\C2-2__MAIN.gms)", 4},
    {R"(SCENES\C2-3\C2-3__MAIN.gms)", 5},
    {R"(SCENES\C2-4\C2-4__MAIN.gms)", 6},
    {R"(SCENES\C3-1\C3-1__MAIN.gms)", 7},
    {R"(SCENES\C3-2a\C3-2a__MAIN.gms)", 8},
    {R"(SCENES\C3-2b\C3-2b__MAIN.gms)", 9},
    {R"(SCENES\C3-3\C3-3__MAIN.gms)", 10},
    {R"(SCENES\C4-1\C4-1__MAIN.gms)", 11},
    {R"(SCENES\C4-2\C4-2__MAIN.gms)", 12},
    {R"(SCENES\C4-3\C4-3__MAIN.gms)", 13},
    {R"(SCENES\C5-1\C5-1__MAIN.gms)", 14},
    {R"(SCENES\C5-2\C5-2__MAIN.gms)", 15},
    {R"(SCENES\C5-3\C5-3__MAIN.gms)", 16},
    {R"(SCENES\C6-1\C6-1__MAIN.gms)", 17},
    {R"(SCENES\C6-2\C6-2__MAIN.gms)", 18},
    {R"(SCENES\C6-3\C6-3__MAIN.gms)", 19},
    {R"(SCENES\C7-1\C7-1__MAIN.gms)", 20},
    {R"(SCENES\C8-1\C8-1__MAIN.gms)", 21},
};

// note: index 0 is for map 2 etc.
const std::vector<int32_t> second_offsets
    = {0x838, 0xB24, 0x8A0, 0x138, 0xB88, 0xBB8, 0xB48, 0xCE8, 0x136C, 0xAD0,
       0xF50, 0x8D4, 0x9EC, 0x400, 0x9EC, 0x644, 0xB08, 0x96C, 0xB00,  0x8};

struct GameStats {
    int32_t headshots;          // 0x208
    int32_t enemies_wounded;    // 0x20C
    int32_t enemies_killed;     // 0x210
    int32_t innocents_wounded;  // 0x214
    int32_t innocents_killed;   // 0x218
    int32_t alerts;             // 0x21C
    int32_t close_encounters;   // 0x220
};

static_assert(sizeof(GameStats) == 28);

// https://docs.google.com/spreadsheets/d/1i6dmzcBROqoJlsQjUGY8wxdqwxt2hXzjB9fPVggTf2k/edit?gid=1074822823#gid=1074822823
const std::vector<std::array<int32_t, 8>> silent_assassin_combinations
    = {{0, 1, 0, 0, 1, 2, 0, 0}, {0, 1, 0, 0, 0, 5, 0, 0},
       {0, 1, 0, 0, 0, 2, 0, 1}, {0, 0, 0, 1, 2, 0, 0, 0},
       {0, 0, 0, 1, 1, 3, 0, 0}, {0, 0, 0, 1, 1, 0, 0, 1},
       {0, 0, 0, 1, 0, 6, 0, 0}, {0, 0, 0, 1, 0, 3, 0, 1},
       {0, 0, 0, 1, 0, 0, 1, 0}, {0, 0, 0, 1, 0, 0, 0, 2},
       {0, 0, 0, 0, 1, 0, 0, 1}, {1, 1, 1, 0, 0, 2, 0, 0},
       {1, 1, 0, 0, 1, 0, 0, 0}, {1, 1, 0, 0, 0, 3, 0, 0},
       {1, 1, 0, 0, 0, 0, 0, 1}, {1, 0, 1, 1, 1, 0, 0, 0},
       {1, 0, 1, 1, 0, 3, 0, 0}, {1, 0, 1, 1, 0, 0, 0, 1},
       {1, 0, 0, 1, 1, 1, 0, 0}, {1, 0, 0, 1, 0, 4, 0, 0},
       {1, 0, 0, 1, 0, 1, 0, 1}, {1, 0, 0, 0, 1, 1, 0, 0},
       {2, 1, 1, 0, 0, 0, 0, 0}, {2, 1, 0, 0, 0, 1, 0, 0},
       {2, 0, 2, 1, 0, 0, 0, 0}, {2, 0, 1, 1, 0, 1, 0, 0},
       {3, 0, 0, 1, 0, 0, 0, 0}};

static bool is_less_or_equal(
    std::array<int32_t, 8> comb1, std::array<int32_t, 8> comb2
) {
    for (int i = 0; i < 8; i++) {
        if (comb1[i] > comb2[i]) return false;
    };
    return true;
}

static SilentAssassin get_silent_assassin(const Stats& stats) {
    std::array<int32_t, 8> stats_comb{
        stats.shots_fired,
        stats.close_encounters,
        stats.headshots,
        stats.alerts,
        stats.enemies_killed,
        stats.enemies_wounded,
        stats.innocents_killed,
        stats.innocents_wounded,
    };
    for (const auto& sa_comb : silent_assassin_combinations) {
        if (is_less_or_equal(stats_comb, sa_comb)) return SilentAssassin::YES;
    };
    return SilentAssassin::NO;
}

void hitman2_silent_assassin::update_slow(
    void* handle, int32_t hook_target_ptr, Stats& stats
) {
    auto scene = read_string(handle, 0x006A6C5C, {0x98, 0xBBB}, 64);
    if (!scene.empty()) spdlog::trace("Scene {}", scene);
    auto iter = scenes.find(scene);
    if (iter != scenes.end()) {
        stats.map = iter->second;
        spdlog::trace("Map {}", stats.map);
    } else {
        stats.map = 0;
        write_int32(handle, hook_target_ptr, 0);
    }
    if (stats.map >= 2) {
        stats.shots_fired = read_int32(handle, hook_target_ptr);
        spdlog::trace("Shots fired {}", stats.shots_fired);
        GameStats game_stats{0};
        if (read_bytes(
                handle,
                0x6A6C50,
                {0x28, second_offsets.at(stats.map - 2), 0x208},
                &game_stats,
                sizeof(game_stats)
            )) {
            spdlog::trace("Headshots {}", game_stats.headshots);
            spdlog::trace("Enemies wounded {}", game_stats.enemies_wounded);
            spdlog::trace("Enemies killed {}", game_stats.enemies_killed);
            spdlog::trace("Innocents wounded {}", game_stats.innocents_wounded);
            spdlog::trace("Innocents killed {}", game_stats.innocents_killed);
            spdlog::trace("Alerts {}", game_stats.alerts);
            spdlog::trace("Close encounters {}", game_stats.close_encounters);
            stats.headshots = game_stats.headshots;
            stats.enemies_wounded = game_stats.enemies_wounded;
            stats.enemies_killed = game_stats.enemies_killed;
            stats.innocents_killed = game_stats.innocents_killed;
            stats.innocents_wounded = game_stats.innocents_wounded;
            stats.alerts = game_stats.alerts;
            stats.close_encounters = game_stats.close_encounters;
            stats.silent_assassin = get_silent_assassin(stats);
        }
    }
}

void hitman2_silent_assassin::update_fast(
    void* handle, int32_t hook_target_ptr, Stats& stats
) {
    if (stats.map > 0) {
        stats.time
            = read_int32(handle, 0x006A6C58, {0x118, 0xB38, 0x8, 0x1084, 0x24})
              * 0.0166666666666666f;  // 1 / 60.0f
    }
}