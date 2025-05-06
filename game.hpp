#pragma once

#include <cinttypes>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "handle.hpp"
#include "stats.hpp"

using GameGui = std::function<void(const Stats &)>;
using GameStats = std::function<void(void *, Stats &)>;

struct GameMethods {
    GameGui gui;            // called every frame (for displaying stats)
    GameStats init;         // called once (for e.g. hooking functions)
    GameStats update_slow;  // called every 0.1 seconds (for slow varying stats)
    GameStats update_fast;  // called every frame (for fast varying stats)
};

struct Game {
    HandlePtr handle;
    std::unordered_map<std::string, int32_t> module_base;
    GameMethods methods;
};

std::optional<Game> find_game();

bool game_is_running(void *process_handle);