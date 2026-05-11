#include "stats.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"

struct Scene {
    int map{0};
    MapStage map_stage{MapStage::pre};
};

// unordered_map for fast lookup
const std::unordered_map<std::string, Scene> scenes = {
    {R"(scenes\mainmenu.gms)", {}},             // main menu
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

bool hitman_contracts::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    static int map_pointer_number = 0;
    auto scene = read_string(
        handle, base_ptrs[0] + 0x39457C, {0xA5, 0xBCD, 0x0}, INT32_MAX, 0x100
    );
    if (!scene) {
        logging::warn("Unable to read scene");
        return false;
    }
    logging::trace("Scene {}", scene.value());
    std::string scene_lower{scene.value()};
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
        logging::error("Unknown scene {}", scene.value());
        stats.map = 0;
    }
    stats.difficulty = read<int32_t>(handle, label_ptrs.at(250)).value_or(0);
    if (stats.map >= 1) {
        auto shots_fired = read<int32_t>(
            handle, base_ptrs[0] + 0x3947A8 + 0x8, {0x13DB}, INT32_MAX
        );
        if (shots_fired) {
            logging::trace("Shots fired {}", shots_fired.value());
            stats.shots_fired.value = shots_fired.value();
        } else {
            logging::warn("Unable to read shots fired");
            stats.shots_fired.value = 0;
        }
        CommonGameStats game_stats{0};
        if (read_bytes(
                handle,
                base_ptrs[0] + 0x3947C0,
                {0xB17},
                INT32_MAX,
                &game_stats,
                sizeof(game_stats)
            )) {
            process_common_game_stats(
                stats.map == 1 ? silent_assassin_combinations_map_1
                               : silent_assassin_combinations,
                game_stats,
                stats
            );
        } else {
            logging::warn("Unable to read game stats");
        }
    }
    return true;
}

constexpr float time_scale = 1.0f / 1024;

bool hitman_contracts::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        auto time
            = read<int32_t>(handle, base_ptrs[0] + 0x39457C, {0x38}, INT32_MAX);
        if (time) stats.time = time.value() * time_scale;
        return time.has_value();
    }
    return true;
}