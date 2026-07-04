#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman_contracts {

inline constexpr auto GAME_NAME = "Hitman: Contracts";

GameGui gui(const settings::Gui& settings, const std::string& version);

}  // namespace hitman_contracts