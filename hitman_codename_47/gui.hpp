#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman_codename_47 {

inline constexpr auto GAME_NAME = "Hitman: Codename 47";

GameGui gui(const settings::Gui& settings, const std::string& version);

}  // namespace hitman_codename_47