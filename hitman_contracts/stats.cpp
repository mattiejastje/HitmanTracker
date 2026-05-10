#include "stats.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"

const std::vector<std::pair<intptr_t, std::vector<intptr_t>>> map_pointers
    = {{0x00393D58, {0x234, 0xBDE}},
       {0x00394598, {0x10, 0x194, 0xC0E}},
       {0x00394598, {0x214, 0xC0E}},
       {0x00394578, {0x1EC0, 0x49FA}},
       {0x00394578, {0x1E00, 0xBC, 0x49FA}},
       {0x00394578, {0x1D80, 0x7C, 0xBC, 0x49FA}},
       {0x00394578, {0x1D00, 0x7C, 0x7C, 0xBC, 0x49FA}},
       {0x0039457C, {0x1E40, 0x49FA}},
       {0x0039457C, {0x1D80, 0xBC, 0x49FA}},
       {0x0039457C, {0x1D00, 0x7C, 0xBC, 0x49FA}},
       {0x0039457C, {0x1C80, 0x7C, 0x7C, 0xBC, 0x49FA}}};

// unordered_map for fast lookup
const std::unordered_map<std::string, int> scenes = {
    {"C01-1_MA", 1},
    {"C01-2_MA", 2},
    {"C02-1_MA", 3},
    {"C03-1_MA", 4},
    {"C06-1_MA", 5},
    {"C06-2_MA", 6},
    {"C07-1_MA", 7},
    {"C08-1_MA", 8},
    {"C08-2_MA", 9},
    {"C08-3_MA", 10},
    {"C08-4_MA", 11},
    {"C09-1_MA", 12},
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
        handle,
        base_ptrs[0] + map_pointers[map_pointer_number].first,
        map_pointers[map_pointer_number].second,
        INT32_MAX,
        8
    );
    if (!scene) {
        // try different pointer on next iteration
        map_pointer_number++;
        if (map_pointer_number > 10) map_pointer_number = 0;
        logging::trace("map pointer number: {}", map_pointer_number);
        return true;
    }
    logging::trace("Scene {}", scene.value());
    auto iter = scenes.find(scene.value());
    if (iter != scenes.end()) {
        stats.map = iter->second;
        logging::trace("Map {}", stats.map);
    } else {
        stats.map = 0;
    }
    stats.map_stage = MapStage::main;  // always render stats
    stats.difficulty = read<int32_t>(handle, label_ptrs.at(250)).value_or(0);
    if (stats.map >= 1) {
        auto shots_fired = read<int32_t>(
            handle, base_ptrs[0] + 0x3947B0, {0xBA0, 0x104, 0x82F}, INT32_MAX
        );
        if (shots_fired) {
            logging::trace("Shots fired {}", shots_fired.value());
            stats.shots_fired.value = shots_fired.value();
        } else {
            logging::warn("Unable to read shots fired");
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