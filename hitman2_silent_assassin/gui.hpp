#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman2_silent_assassin {

inline constexpr auto GAME_NAME = "Hitman 2: Silent Assassin";

GameGui gui(const settings::Gui& settings, const std::string& version);

}  // namespace hitman2_silent_assassin
