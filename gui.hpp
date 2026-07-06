#pragma once

#include "game_info.hpp"
#include "settings.hpp"

int gui_run(
    const std::vector<GameInfo>& registry, settings::Settings& settings
);
