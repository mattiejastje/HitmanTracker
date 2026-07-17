#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman_blood_money {

GameGui gui(const settings::Gui& settings, const settings::HBM& hbm, const std::string& version);

}  // namespace hitman_blood_money