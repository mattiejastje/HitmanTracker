#pragma once

#include <vector>

#include "../game_info.hpp"
#include "../settings.hpp"

namespace hitman2_silent_assassin {

void register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
);

}