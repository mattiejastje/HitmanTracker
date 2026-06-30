#pragma once

#include <any>
#include <cstdint>
#include <filesystem>
#include <optional>

#include "game_info.hpp"

struct Game {
    std::filesystem::path exe_path;
    std::shared_ptr<void> handle;
    BasePtrs base_ptrs;
    GameMethods methods;
    HookPtr hook;
    // remote_state stored here to avoid re-allocating on every update
    std::any remote_state;
    // game-specific storage for statistics
    std::any stats;
};

std::optional<Game> find_game();
