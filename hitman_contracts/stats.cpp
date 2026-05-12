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

struct Scene {
    int map{0};
    MapStage map_stage{MapStage::pre};
};

// unordered_map for fast lookup
const std::unordered_map<std::string, Scene> scenes = {
    {R"(scenes\mainmenu.gms)", {}},  // main menu
    {R"(scenes\alllevels\logos.gms)", {}},
    {R"(scenes\alllevels\levelmenu.gms)", {}},  // level menu
    {R"(scenes\inventorymenu.gms)", {}},        // inventory menu
    {R"(scenes\c00-1\c00-1_load.gms)", {}},     // training load
    {R"(scenes\c00-1\c00-1_main.gms)", {}},     // training main
    {R"(scenes\c01-1\c01-1_load.gms)", {1}},
    {R"(scenes\c01-1\c01-1_main.gms)", {1, MapStage::main}},
    {R"(scenes\c01-2\c01-2_load.gms)", {2}},
    {R"(scenes\c01-2\c01-2_main.gms)", {2, MapStage::main}},
    {R"(scenes\c02-1\c02-1_load.gms)", {3}},
    {R"(scenes\c02-1\c02-1_main.gms)", {3, MapStage::main}},
    {R"(scenes\c03-1\c03-1_load.gms)", {4}},
    {R"(scenes\c03-1\c03-1_main.gms)", {4, MapStage::main}},
    {R"(scenes\c06-1\c06-1_load.gms)", {5}},
    {R"(scenes\c06-1\c06-1_main.gms)", {5, MapStage::main}},
    {R"(scenes\c06-2\c06-2_load.gms)", {6}},
    {R"(scenes\c06-2\c06-2_main.gms)", {6, MapStage::main}},
    {R"(scenes\c07-1\c07-1_load.gms)", {7}},
    {R"(scenes\c07-1\c07-1_main.gms)", {7, MapStage::main}},
    {R"(scenes\c08-1\c08-1_load.gms)", {8}},
    {R"(scenes\c08-1\c08-1_main.gms)", {8, MapStage::main}},
    {R"(scenes\c08-2\c08-2_load.gms)", {9}},
    {R"(scenes\c08-2\c08-2_main.gms)", {9, MapStage::main}},
    {R"(scenes\c08-3\c08-3_load.gms)", {10}},
    {R"(scenes\c08-3\c08-3_main.gms)", {10, MapStage::main}},
    {R"(scenes\c08-4\c08-4_load.gms)", {11}},
    {R"(scenes\c08-4\c08-4_main.gms)", {11, MapStage::main}},
    {R"(scenes\c09-1\c09-1_load.gms)", {12}},
    {R"(scenes\c09-1\c09-1_main.gms)", {12, MapStage::main}},
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
    auto aggression = static_cast<int32_t>(sigmoid * 200.0f);
    if (aggression <= 2) {
        // force minimum of 3 if any civilians were hurt
        // (i.e. no silent assassin)
        if (stats.innocents_killed > 0 || stats.innocents_wounded > 0) return 3;
        // force minimum of 3 if close encounters in mission C01-1
        // (i.e. no silent assassin)
        if (map == 1 && stats.close_encounters > 0) return 3;
    } else if (stats.close_encounters == 0 && stats.enemies_killed == 0
               && stats.enemies_wounded == 0 && stats.innocents_wounded == 0
               && stats.innocents_killed == 0 && stats.headshots == 0)
        // special case: nothing happened besides distraction shots
        return 2;
    // no special case
    return aggression;
}

static int32_t measure_stealth(const CommonGameStats& stats) {
    auto raw = stats.alerts + stats.close_encounters;
    auto power = powf(0.9f, static_cast<float>(raw)) * 100.0f;
    return static_cast<int32_t>(power);
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
    std::string scene_lower{scene};
    std::transform(
        scene_lower.begin(),
        scene_lower.end(),
        scene_lower.begin(),
        [](char& c) { return std::tolower(c); }
    );
    auto iter = scenes.find(scene_lower);
    if (iter != scenes.end()) {
        stats.map = iter->second.map;
        stats.map_stage = iter->second.map_stage;
        logging::trace("Map {}", stats.map);
    } else {
        logging::error("Unknown scene {}", scene);
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