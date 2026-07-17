#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman_contracts {

GameGui gui(const settings::Gui& settings, const std::string& version);

}  // namespace hitman_contracts