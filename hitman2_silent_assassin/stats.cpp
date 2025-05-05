#include "stats.hpp"

#include <unordered_map>

#include "../read_process_memory.hpp"

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

void update_slow_hitman2_silent_assassin(
    const HandlePtr& handle, Stats& stats
) {
    auto scene = read_string(handle, 0x006A6C5C, {0x98, 0xBBB}, 64);
    spdlog::trace("Scene {}", scene);
    auto iter = scenes.find(scene);
    if (iter != scenes.end()) {
        stats.map = iter->second;
        spdlog::trace("Map {}", stats.map);
    } else {
        stats.map = 0;
    };
}

void update_fast_hitman2_silent_assassin(
    const HandlePtr& handle, Stats& stats
) {
    if (stats.map > 0) {
        stats.time
            = read_uint32(handle, 0x006A6C58, {0x118, 0xB38, 0x8, 0x1084, 0x24})
              * 0.0166666666666666f;  // 1 / 60.0f
    }
}
