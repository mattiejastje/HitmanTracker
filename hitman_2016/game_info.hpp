#pragma once

#include <vector>

#include "../game_info.hpp"
#include "../settings.hpp"

namespace hitman_2016 {

void register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
);

}