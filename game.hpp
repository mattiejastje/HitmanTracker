#pragma once

#include <cinttypes>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "mem/handle.hpp"
#include "hook.hpp"
#include "stats.hpp"

using GameHook = std::function<HookPtr(std::shared_ptr<void>)>;
using GameGui = std::function<void(const Stats &)>;
using GameStats = std::function<void(void *, int32_t, Stats &)>;

struct GameMethods {
    GameGui gui;            // called every frame for displaying stats
    GameHook hook;          // called once for hooking functions
    GameStats update_slow;  // called every 0.1 seconds for slow varying stats
    GameStats update_fast;  // called every frame for fast varying stats
};

struct Game {
    std::shared_ptr<void> handle;
    std::unordered_map<std::string, int32_t> module_base;
    GameMethods methods;
};

std::optional<Game> find_game();

bool game_is_running(void *process_handle);