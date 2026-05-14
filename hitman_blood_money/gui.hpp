#pragma once

#include "../game.hpp"

namespace hitman_blood_money {

inline constexpr auto GAME_NAME = "Hitman: Blood Money";

void gui(const settings::Gui& settings, const Fonts& fonts, const Stats& stats);

}  // namespace hitman_blood_money