#pragma once

#include "../game.hpp"
#include "version.hpp"

namespace hitman_contracts {

GameStatsSlow update_slow(Version version);

GameStatsFast update_fast(Version version);

}  // namespace hitman_contracts
