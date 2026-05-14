#pragma once

#include "../game.hpp"

namespace hitman2_silent_assassin {

inline constexpr auto GAME_NAME = "Hitman 2: Silent Assassin";

void gui(const settings::Gui& settings, const Fonts& fonts, const Stats& stats);

}
