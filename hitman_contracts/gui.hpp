#pragma once

#include "../game.hpp"

namespace hitman_contracts {

inline constexpr auto GAME_NAME = "Hitman: Contracts";

void gui(const settings::Gui& settings, const Fonts& fonts, const Stats& stats);

}