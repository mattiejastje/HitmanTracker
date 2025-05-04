#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "process_handle.hpp"
#include "stats.hpp"

using GameGui = std::function<void(const Stats&)>;
using GameStats = std::function<void(const ProcessHandlePtr&, Stats&)>;

struct Game {
    ProcessHandlePtr handle;
    GameGui gui;
    GameStats stats;
};

std::optional<Game> find_game();

bool game_is_running(const ProcessHandlePtr& process_handle);
