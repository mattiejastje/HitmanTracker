#pragma once

#include <vector>

#include "../game_info.hpp"
#include "../settings.hpp"

namespace hitman_absolution {

void register_game_info(
    std::vector<GameInfo>& registry,
    const settings::Gui& settings,
    const settings::HMA& hma
);

}