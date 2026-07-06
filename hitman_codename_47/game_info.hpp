#pragma once

#include <vector>

#include "../game_info.hpp"
#include "../settings.hpp"

namespace hitman_codename_47 {

void register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
);

}