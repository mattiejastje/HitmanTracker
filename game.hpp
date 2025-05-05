#pragma once

#include <functional>
#include <optional>

#include "process_handle.hpp"
#include "stats.hpp"

using GameHook = std::function<void(const ProcessHandlePtr&, Stats&)>;
using GameGui = std::function<void(const Stats&)>;
using GameStats = std::function<void(const ProcessHandlePtr&, Stats&)>;

struct GameMethods {
    GameHook hook;
    GameGui gui;
    GameStats stats;
};

struct Game {
    ProcessHandlePtr handle;
    GameMethods methods;
};

std::optional<Game> find_game();

bool game_is_running(const ProcessHandlePtr& process_handle);
