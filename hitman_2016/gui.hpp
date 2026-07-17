#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman_2016 {

GameGui gui(const settings::Gui& settings, const std::string& version);

}  // namespace hitman_2016