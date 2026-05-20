#include "stats.hpp"

#include <cmath>
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

// https://docs.google.com/spreadsheets/d/1JgNscwEak6pR5qMcUzjRlGh34IG4aZJ6id9V8rahL18/edit?gid=1089548412#gid=1089548412
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

static auto measure_aggression(
    const CommonGameStats& stats, int32_t shots_fired, int32_t map
) {
    auto value = 3 * stats.innocents_wounded + 6 * stats.innocents_killed
                 + stats.enemies_wounded + 3 * stats.enemies_killed
                 + 2 * shots_fired + stats.headshots + stats.close_encounters;
    // static_cast to round down towards zero (value is non-negative)
    auto aggression = 100 * std::tanh(0.005 * value);
    if (aggression <= 2) {
        // cap min at 3 if innocents hurt or close encounter on 1st map
        if (stats.innocents_killed > 0 || stats.innocents_wounded > 0)
            return 3.0;
        if (map == 1 && stats.close_encounters > 0) return 3.0;
    } else if (stats.close_encounters == 0 && stats.enemies_killed == 0
               && stats.enemies_wounded == 0 && stats.innocents_killed == 0
               && stats.innocents_wounded == 0 && stats.headshots == 0) {
        // cap max at 2 in distraction shots only scenario
        return 2.0;
    }
    // no special case
    return aggression;
}

static auto measure_stealth(const CommonGameStats& stats) {
    auto value = stats.alerts + stats.close_encounters;
    return 100 * std::pow(0.9, value);
}

bool hitman_contracts::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    const RemoteValue<structs::THitmanContracts, uint32_t> remote_game{
        static_cast<uint32_t>(base_ptrs.at(0))
    };
    MemoryReader<uint32_t> reader{handle};
    auto tracer
        = mempeep::LogTracer{MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS};
    if (!mempeep::read(remote_game, reader, tracer, game)) return false;
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
            // TODO rely on stealth & aggression instead to compute rating,
            // and to identify at-risk and violating variables
            process_common_game_stats(
                stats.map == 1 ? silent_assassin_combinations_map_1
                               : silent_assassin_combinations,
                game_stats,
                stats
            );
            auto stealth = measure_stealth(game_stats);
            stats.stealth
                = {stealth,
                   stealth >= 84.999999 ? Status::YELLOW : Status::RED};
            auto aggression = measure_aggression(
                game_stats, stats.shots_fired.value, stats.map
            );
            stats.aggression
                = {aggression,
                   aggression <= 2.999999 ? Status::YELLOW : Status::RED};
#ifndef NDEBUG
            // validate the two methods (to be removed when confirmed stable)
            bool is_sa_1 = (stats.rating.status == Status::GREEN);
            bool is_sa_2 = (stealth >= 84.999999 && aggression <= 2.999999);
            if (is_sa_1 != is_sa_2) {
                logging::error(
                    "silent assassin status stealth/aggression mismatch"
                );
            }
#endif
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
        const auto& base_ptr = base_ptrs.at(0);
        auto game_ticks
            = read<int32_t>(handle, base_ptr + 0x39457C, {0x38}, INT32_MAX);
        if (game_ticks) stats.time = game_ticks.value() * seconds_per_tick;
        return game_ticks.has_value();
    }
    return true;
}