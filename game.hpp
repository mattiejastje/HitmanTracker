#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

constexpr auto MAX_STATS_VALUES = 20;

enum SilentAssassin {
    YES,
    MAYBE,
    NO
};

struct Stats {
    float time;
    int map;
    SilentAssassin silent_assassin;
    int values[MAX_STATS_VALUES];
};

struct ProcessHandleDeleter {
    void operator()(void* process_handle) const;
};

using ProcessHandlePtr = std::unique_ptr<void, ProcessHandleDeleter>;
using GameGui = std::function<void(const Stats&)>;
using GameStats = std::function<void(const ProcessHandlePtr&, Stats&)>;

struct Game {
    ProcessHandlePtr handle;
    GameGui gui;
    GameStats stats;
};

std::optional<Game> find_game();

bool game_is_running(const ProcessHandlePtr& process_handle);
