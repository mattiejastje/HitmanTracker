#pragma once

#include "../game.hpp"

namespace hitman_absolution {

inline constexpr auto GAME_NAME = "Hitman: Absolution";

void gui(const settings::Gui& settings, const Fonts& fonts, const Stats& stats);

}  // namespace hitman_absolution