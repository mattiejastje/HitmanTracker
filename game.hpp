#pragma once

#include <functional>
#include <optional>

#include "handle.hpp"
#include "stats.hpp"

using GameGui = std::function<void(const Stats&)>;
using GameStats = std::function<void(const HandlePtr&, Stats&)>;

struct GameMethods {
    GameGui gui;            // called every frame (for displaying stats)
    GameStats init;         // called once (for e.g. hooking functions)
    GameStats update_slow;  // called every 0.1 seconds (for slow varying stats)
    GameStats update_fast;  // called every frame (for fast varying stats)
};

struct Game {
    HandlePtr handle;
    GameMethods methods;
};

std::optional<Game> find_game();

bool game_is_running(const HandlePtr& process_handle);
