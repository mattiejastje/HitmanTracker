#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include "base_ptrs.hpp"
#include "gui/ui.hpp"
#include "hook.hpp"
#include "settings.hpp"
#include "stats.hpp"

using GameGui
    = std::function<void(const settings::Gui &, const Fonts &, const Stats &)>;

using GameStats
    = std::function<bool(void *, const BasePtrs &, const LabelPtrs &, Stats &)>;

struct GameMethods {
    GameGui gui;            // called every frame for displaying stats
    GameStats update_slow;  // called every 0.1 seconds for slow varying stats
    GameStats update_fast;  // called every frame for fast varying stats
};

struct Game {
    std::shared_ptr<void> handle;
    BasePtrs base_ptrs;
    GameMethods methods;
    HookPtr hook;
};

std::optional<Game> find_game();
