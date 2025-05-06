#pragma once

#include "../handle.hpp"
#include "../stats.hpp"

namespace hitman2_silent_assassin {

void update_slow(void *handle, Stats& stats);
void update_fast(void *handle, Stats& stats);

}  // namespace hitman2_silent_assassin
