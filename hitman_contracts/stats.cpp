#include "stats.hpp"

#include <algorithm>
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"
#include "structs.hpp"

// unordered_map for fast lookup
const std::unordered_map<std::string, int> scenes = {
    {R"(SCENES\C01-1\C01-1_MAIN.gms)", 1},
    {R"(SCENES\C01-2\C01-2_MAIN.gms)", 2},
    {R"(SCENES\C02-1\C02-1_MAIN.gms)", 3},
    {R"(SCENES\C03-1\C03-1_MAIN.gms)", 4},
    {R"(SCENES\C06-1\C06-1_MAIN.gms)", 5},
    {R"(SCENES\C06-2\C06-2_MAIN.gms)", 6},
    {R"(SCENES\C07-1\C07-1_MAIN.gms)", 7},
    {R"(SCENES\C08-1\C08-1_MAIN.gms)", 8},
    {R"(SCENES\C08-2\C08-2_MAIN.gms)", 9},
    {R"(SCENES\C08-3\C08-3_MAIN.gms)", 10},
    {R"(SCENES\C08-4\C08-4_MAIN.gms)", 11},
    {R"(SCENES\C09-1\C09-1_MAIN.gms)", 12},
};

// https://docs.google.com/spreadsheets/d/1i6dmzcBROqoJlsQjUGY8wxdqwxt2hXzjB9fPVggTf2k/edit?gid=1074822823#gid=1074822823
const std::vector<StatsArray> silent_assassin_combinations
    = {{999, 0, 999, 1, 0, 0, 0, 0},
       {2, 1, 1, 0, 0, 0, 0, 0},
       {2, 1, 0, 0, 0, 1, 0, 0},
       {2, 0, 1, 1, 0, 1, 0, 0},
       {2, 0, 0, 0, 0, 2, 0, 0},
       {1, 1, 1, 0, 0, 2, 0, 0},
       {1, 1, 0, 0, 1, 0, 0, 0},
       {1, 1, 0, 0, 0, 3, 0, 0},
       {1, 0, 1, 1, 1, 0, 0, 0},
       {1, 0, 1, 1, 0, 3, 0, 0},
       {1, 0, 0, 1, 1, 1, 0, 0},
       {1, 0, 0, 1, 0, 4, 0, 0},
       {0, 1, 0, 0, 1, 2, 0, 0},
       {0, 1, 0, 0, 0, 5, 0, 0},
       {0, 0, 0, 1, 1, 3, 0, 0},
       {0, 0, 0, 1, 2, 0, 0, 0},
       {0, 0, 0, 1, 0, 6, 0, 0}};

const std::vector<StatsArray> silent_assassin_combinations_map_1
    = {{999, 0, 0, 1, 0, 0, 0, 0},
       {2, 0, 0, 0, 0, 2, 0, 0},
       {1, 0, 1, 1, 1, 0, 0, 0},
       {1, 0, 0, 1, 1, 1, 0, 0},
       {1, 0, 0, 1, 0, 4, 0, 0},
       {0, 0, 0, 1, 1, 3, 0, 0},
       {0, 0, 0, 1, 2, 0, 0, 0},
       {0, 0, 0, 1, 0, 6, 0, 0}};

// global to avoid allocating large object on stack
static hitman_contracts::structs::HitmanContracts game{};

static int32_t measure_aggression(
    const CommonGameStats& stats, int32_t shots_fired, int32_t map
) {
    auto raw = 3 * stats.innocents_wounded + 6 * stats.innocents_killed
               + 3 * stats.enemies_killed + stats.enemies_wounded
               + 2 * shots_fired + stats.headshots + stats.close_encounters;
    auto sigmoid = 1.0f / (1.0f + exp(-0.01f * raw)) - 0.5f;
    auto aggression = static_cast<int32_t>(200 * sigmoid);
    if (aggression <= 2) {
        // force minimum of 3 if any civilians were hurt
        // (i.e. no silent assassin)
        if (stats.innocents_killed > 0 || stats.innocents_wounded > 0) return 3;
        // force minimum of 3 if close encounters in mission C01-1
        // (i.e. no silent assassin)
        if (map == 1 && stats.close_encounters > 0) return 3;
    } else if (stats.close_encounters == 0 && stats.enemies_killed == 0
               && stats.enemies_wounded == 0 && stats.innocents_killed == 0
               && stats.innocents_wounded == 0 && stats.headshots == 0)
        // special case: nothing happened besides distraction shots
        return 2;
    // no special case
    return aggression;
}

static int32_t measure_stealth(const CommonGameStats& stats) {
    auto raw = stats.alerts + stats.close_encounters;
    auto stealth = 100 * powf(0.9f, static_cast<float>(raw));
    return static_cast<int32_t>(stealth);
}

bool hitman_contracts::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    MemoryReader<uint32_t> reader{handle};
    auto tracer
        = mempeep::LogTracer{MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS};
    if (!mempeep::read<structs::THitmanContracts>(
            base_ptrs[0], reader, tracer, game
        ))
        return false;
    const auto& scene = game.engine.scene_manager.scene_name.text;
    logging::trace("Scene {}", scene);
    auto iter = scenes.find(scene);
    if (iter != scenes.end()) {
        stats.map = iter->second;
        stats.map_stage = MapStage::main;
        logging::trace("Map {}", stats.map);
    } else {
        logging::trace("Unhandled scene {}", scene);
        stats.map = 0;
    }
    stats.difficulty = read<int32_t>(handle, label_ptrs.at(250)).value_or(0);
    if (stats.map >= 1) {
        const auto& player_data = game.player.data;
        if (player_data) {
            stats.shots_fired.value = player_data->shots_fired;
        } else {
            stats.shots_fired.value = 0;
        }
        const auto& player_stats = game.player.stats;
        CommonGameStats game_stats{0};
        if (player_stats) {
            game_stats.headshots = player_stats->headshots;
            game_stats.enemies_wounded = player_stats->enemies_wounded;
            game_stats.enemies_killed = player_stats->enemies_killed;
            game_stats.innocents_wounded = player_stats->innocents_wounded;
            game_stats.innocents_killed = player_stats->innocents_killed;
            game_stats.alerts = player_stats->alerts;
            game_stats.close_encounters = player_stats->close_encounters;
            process_common_game_stats(
                stats.map == 1 ? silent_assassin_combinations_map_1
                               : silent_assassin_combinations,
                game_stats,
                stats
            );
            auto stealth = measure_stealth(game_stats);
            stats.stealth
                = {stealth, stealth >= 85 ? Status::YELLOW : Status::RED};
            auto aggression = measure_aggression(
                game_stats, stats.shots_fired.value, stats.map
            );
            stats.aggression
                = {aggression, aggression <= 2 ? Status::YELLOW : Status::RED};
        }
    }
    return true;
}

constexpr float seconds_per_tick = 1.0f / 1024;

bool hitman_contracts::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        auto game_ticks
            = read<int32_t>(handle, base_ptrs[0] + 0x39457C, {0x38}, INT32_MAX);
        if (game_ticks) stats.time = game_ticks.value() * seconds_per_tick;
        return game_ticks.has_value();
    }
    return true;
}