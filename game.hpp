#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>

#include "base_ptrs.hpp"
#include "gui/ui.hpp"
#include "hook.hpp"
#include "settings.hpp"
#include "stats.hpp"

using GameGui
    = std::function<void(const settings::Gui &, const Fonts &, const Stats &)>;

using GameHook
    = std::function<HookPtr(std::shared_ptr<void>, const BasePtrs &)>;

using GameHookReady = std::function<bool(void *, const BasePtrs &)>;

using GameStatsSlow = std::function<bool(
    const std::filesystem::path &,
    void *,
    const BasePtrs &,
    const LabelPtrs &,
    Stats &
)>;

using GameStatsFast
    = std::function<bool(void *, const BasePtrs &, const LabelPtrs &, Stats &)>;

struct GameMethods {
    GameGui gui;                // called every frame for displaying stats
    GameHook hook;              // called once when we are ready to hook
    GameHookReady hook_ready;   // called to check if we are ready to hook
    GameStatsSlow update_slow;  // called every 0.1 seconds
    GameStatsFast update_fast;  // called every frame
};

struct Game {
    std::filesystem::path exe_path;
    std::shared_ptr<void> handle;
    BasePtrs base_ptrs;
    GameMethods methods;
    HookPtr hook;
};

std::optional<Game> find_game();
