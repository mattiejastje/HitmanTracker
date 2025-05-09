#pragma once

#include <cinttypes>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "hook.hpp"
#include "mem/handle.hpp"
#include "settings.hpp"
#include "stats.hpp"

struct ImFont;

// imgui font for each text style in settings::Gui
struct Fonts {
    ImFont *title;
    ImFont *map;
    ImFont *time;
    ImFont *rating_bad;
    ImFont *rating_good;
    ImFont *rating_maybe;
    ImFont *label;
    ImFont *value;
};

using GameHook = std::function<HookPtr(std::shared_ptr<void>)>;
using GameGui
    = std::function<void(const settings::Gui &, const Fonts &, const Stats &)>;
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