#pragma once

#include "../game.hpp"

namespace hitman_blood_money {

inline constexpr auto GAME_NAME = "Hitman: Blood Money";

GameGui gui(const settings::Gui& settings, const settings::HBM& hbm, const std::string& version);

}  // namespace hitman_blood_money