#pragma once

#include "../game.hpp"

namespace hitman_contracts {

inline constexpr auto GAME_NAME = "Hitman: Contracts";

GameGui gui(const settings::Gui& settings);

}