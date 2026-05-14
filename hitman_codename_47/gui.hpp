#pragma once

#include "../game.hpp"

namespace hitman_codename_47 {

inline constexpr auto GAME_NAME = "Hitman: Codename 47";

void gui(const settings::Gui& settings, const Fonts& fonts, const Stats& stats);

}  // namespace hitman_codename_47