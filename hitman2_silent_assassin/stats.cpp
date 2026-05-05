#include "stats.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../hitman_common/stats.hpp"
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
const std::vector<intptr_t> second_offsets
    = {0x838, 0xB24, 0x8A0, 0x138, 0xB88, 0xBB8, 0xB48, 0xCE8, 0x136C, 0xAD0,
       0xF50, 0x8D4, 0x9EC, 0x400, 0x9EC, 0x644, 0xB08, 0x96C, 0xB00,  0x8};

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

bool hitman2_silent_assassin::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    auto scene = read_string(
        handle, base_ptrs[0] + 0x2A6C5C, {0x98, 0xBBB}, INT32_MAX, 64
    );
    if (scene) logging::trace("Scene {}", scene.value());
    auto iter = scene ? scenes.find(scene.value()) : scenes.end();
    stats.map = iter != scenes.end() ? iter->second : 0;
    // the hook does not reset shots fired pointer on mission load/reload
    // so we reset the pointer when the mission has just started i.e. during the
    // first second (assuming no shots in first second of the mission...)
    if (stats.time < 1.0f) write<int32_t>(handle, label_ptrs.at(150), 0);
    stats.map_stage = MapStage::main;  // always render stats
    stats.difficulty = read<int32_t>(handle, label_ptrs.at(250)).value_or(0);
    if (stats.map >= 2) {
        intptr_t shots_fired_ptr
            = read<int32_t>(handle, label_ptrs.at(150)).value_or(0);
        stats.shots_fired.value
            = shots_fired_ptr
                  ? read<int32_t>(handle, shots_fired_ptr + 0x11C7).value_or(0)
                  : 0;
        CommonGameStats game_stats{0};
        // note: reading game_stats may fail if we are reloading
        read_bytes(
            handle,
            base_ptrs[0] + 0x2A6C50,
            {0x28, second_offsets.at(stats.map - 2), 0x208},
            INT32_MAX,
            &game_stats,
            sizeof(game_stats)
        );
        process_common_game_stats(
            silent_assassin_combinations, game_stats, stats
        );
    } else {
        stats.rating = {"Unrated", Status::GREEN};
    }
    return true;
}

bool hitman2_silent_assassin::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        auto time = read<int32_t>(
            handle,
            base_ptrs[0] + 0x2A6C58,
            {0x118, 0xB38, 0x8, 0x1084, 0x24},
            INT32_MAX
        );
        if (time) stats.time = time.value() * 0.0166666666666666f;  // 1 / 60.0f
        return time.has_value();
    }
    return true;
}