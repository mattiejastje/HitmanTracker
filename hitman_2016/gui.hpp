#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman_2016 {

inline constexpr auto GAME_NAME = "Hitman (2016)";

GameGui gui(const settings::Gui& settings, const std::string& version);

}  // namespace hitman_2016