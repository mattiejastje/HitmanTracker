#pragma once

#include <filesystem>

#include "../base_ptrs.hpp"
#include "../game_info.hpp"
#include "../label_ptrs.hpp"
#include "../stats.hpp"
#include "version.hpp"

namespace hitman_absolution {

GameStatsSlow update_slow(const settings::HMA& hma, Version version);

GameStatsFast update_fast(Version version);

}  // namespace hitman_absolution
