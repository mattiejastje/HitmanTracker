#pragma once

#include "../game.hpp"

namespace hitman_absolution {

inline constexpr auto GAME_NAME = "Hitman: Absolution";

GameGui gui(const settings::Gui& settings, const std::string& version);

}  // namespace hitman_absolution