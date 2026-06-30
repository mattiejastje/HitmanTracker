#pragma once

#include "../game.hpp"
#include "version.hpp"

namespace hitman2_silent_assassin {

GameStatsSlow update_slow(Version version);

GameStatsFast update_fast(Version version);


}  // namespace hitman2_silent_assassin
