#include "stats.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <vector>

#include "../logging.hpp"
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

using StatsArray = std::array<int32_t, 8>;

static StatsArray stats_as_array(const Stats& stats) {
    return StatsArray{
        stats.shots_fired.value,
        stats.close_encounters.value,
        stats.headshots.value,
        stats.alerts.value,
        stats.enemies_killed.value,
        stats.enemies_wounded.value,
        stats.innocents_killed.value,
        stats.innocents_wounded.value,
    };
}

// https://docs.google.com/spreadsheets/d/1i6dmzcBROqoJlsQjUGY8wxdqwxt2hXzjB9fPVggTf2k/edit?gid=1074822823#gid=1074822823
const std::vector<StatsArray> silent_assassin_combinations
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

using StatsExcess = StatsArray;

// excess values above a silent assassin combination
// any values > 0 mean no silent assassin
static StatsExcess get_subtract(
    const StatsArray& stats, const StatsArray& stats_sa
) {
    StatsArray result{};
    for (int i = 0; i < 8; i++) {
        result[i] = stats[i] - stats_sa[i];
    };
    return result;
}

struct Excess {
    int32_t maximum;         // the maximum excess
    std::set<int> positive;  // strictly positive excess stats ("red")
};

static Excess get_arg_max(const StatsExcess& excess) {
    auto maximum = excess[0];
    for (int i = 1; i < 8; i++) {
        auto value = excess[i];
        if (maximum < value) maximum = value;
    };
    Excess arg_max{maximum, {}};
    for (int i = 0; i < 8; i++) {
        if (excess[i] > 0) arg_max.positive.insert(i);
    }
    return arg_max;
}

static std::vector<Excess> get_arg_maxs(const StatsArray& stats) {
    std::vector<Excess> arg_maxs{};
    for (auto& stats_sa : silent_assassin_combinations) {
        arg_maxs.push_back(get_arg_max(get_subtract(stats, stats_sa)));
    }
    return arg_maxs;
}

static Excess get_arg_min(const std::vector<Excess>& arg_maxs) {
    assert(!arg_maxs.empty());
    Excess arg_min{arg_maxs[0]};
    for (auto& arg_max : arg_maxs) {
        if (arg_min.maximum > arg_max.maximum) {
            arg_min = arg_max;
        } else if (arg_min.maximum == arg_max.maximum) {
            for (auto i : arg_max.positive) arg_min.positive.insert(i);
        }
    }
    return arg_min;
};

static bool is_at_risk(const StatsArray& stats, int index) {
    StatsArray stats_inc = stats;
    stats_inc[index] += 1;
    return get_arg_min(get_arg_maxs(stats_inc)).positive.contains(index);
}

static Status get_status(
    const StatsArray& stats, int index, const Excess& arg_min
) {
    return arg_min.positive.contains(index) ? Status::RED
           : is_at_risk(stats, index)       ? Status::YELLOW
                                            : Status::GREEN;
};

void hitman2_silent_assassin::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
) {
    auto scene
        = read_string(handle, base_ptrs[0] + 0x2A6C5C, {0x98, 0xBBB}, 64);
    if (!scene) return;
    logging::trace("Scene {}", scene.value());
    auto iter = scenes.find(scene.value());
    if (iter != scenes.end()) {
        stats.map = iter->second;
        logging::trace("Map {}", stats.map);
    } else {
        stats.map = 0;
    }
    if (stats.map >= 2) {
        auto shots_fired = read<int32_t>(
            handle, base_ptrs[0] + 0x092894, {0x2E0, 0x4, 0x11C7}
        );
        if (shots_fired) {
            logging::trace("Shots fired {}", shots_fired.value());
            stats.shots_fired.value = shots_fired.value();
        } else {
            logging::warn("Unable to read shots fired");
        }
        GameStats game_stats{0};
        if (read_bytes(
                handle,
                base_ptrs[0] + 0x2A6C50,
                {0x28, second_offsets.at(stats.map - 2), 0x208},
                &game_stats,
                sizeof(game_stats)
            )) {
            logging::trace("Headshots {}", game_stats.headshots);
            logging::trace("Enemies wounded {}", game_stats.enemies_wounded);
            logging::trace("Enemies killed {}", game_stats.enemies_killed);
            logging::trace(
                "Innocents wounded {}", game_stats.innocents_wounded
            );
            logging::trace("Innocents killed {}", game_stats.innocents_killed);
            logging::trace("Alerts {}", game_stats.alerts);
            logging::trace("Close encounters {}", game_stats.close_encounters);
            stats.headshots.value = game_stats.headshots;
            stats.enemies_wounded.value = game_stats.enemies_wounded;
            stats.enemies_killed.value = game_stats.enemies_killed;
            stats.innocents_killed.value = game_stats.innocents_killed;
            stats.innocents_wounded.value = game_stats.innocents_wounded;
            stats.alerts.value = game_stats.alerts;
            stats.close_encounters.value = game_stats.close_encounters;
            auto stats_arr = stats_as_array(stats);
            auto arg_min = get_arg_min(get_arg_maxs(stats_arr));
            stats.shots_fired.status = get_status(stats_arr, 0, arg_min);
            stats.close_encounters.status = get_status(stats_arr, 1, arg_min);
            stats.headshots.status = get_status(stats_arr, 2, arg_min);
            stats.alerts.status = get_status(stats_arr, 3, arg_min);
            stats.enemies_killed.status = get_status(stats_arr, 4, arg_min);
            stats.enemies_wounded.status = get_status(stats_arr, 5, arg_min);
            stats.innocents_killed.status = get_status(stats_arr, 6, arg_min);
            stats.innocents_wounded.status = get_status(stats_arr, 7, arg_min);
            stats.silent_assassin
                = arg_min.maximum <= 0 ? Status::GREEN : Status::RED;
        } else {
            logging::warn("Unable to read game stats");
        }
    }
}

void hitman2_silent_assassin::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
) {
    if (stats.map > 0) {
        stats.time = read<int32_t>(
                         handle,
                         base_ptrs[0] + 0x2A6C58,
                         {0x118, 0xB38, 0x8, 0x1084, 0x24}
                     )
                         .value_or(stats.time)
                     * 0.0166666666666666f;  // 1 / 60.0f
    }
}