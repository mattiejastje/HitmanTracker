#pragma once

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
};

std::optional<Game> find_game();
