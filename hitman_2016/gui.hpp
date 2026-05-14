#pragma once

#include "../game.hpp"

namespace hitman_2016 {

inline constexpr auto GAME_NAME = "Hitman (2016)";

void gui(const settings::Gui& settings, const Fonts& fonts, const Stats& stats);

}