#pragma once

#include <functional>
#include <optional>

#include "handle.hpp"
#include "stats.hpp"

using GameHook = std::function<void(const HandlePtr&, Stats&)>;
using GameGui = std::function<void(const Stats&)>;
using GameStats = std::function<void(const HandlePtr&, Stats&)>;

struct GameMethods {
    GameHook hook;
    GameGui gui;
    GameStats stats;
};

struct Game {
    HandlePtr handle;
    GameMethods methods;
};

std::optional<Game> find_game();

bool game_is_running(const HandlePtr& process_handle);
