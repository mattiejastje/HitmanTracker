#pragma once

#include "../game.hpp"
#include "../settings.hpp"

namespace hitman_absolution {

GameGui gui(
    const settings::Gui& settings,
    const settings::HMA& hma,
    const std::string& version
);

}  // namespace hitman_absolution