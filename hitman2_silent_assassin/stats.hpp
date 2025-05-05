#pragma once

#include "../handle.hpp"
#include "../stats.hpp"

namespace hitman2_silent_assassin {

void update_slow(const HandlePtr& handle, Stats& stats);
void update_fast(const HandlePtr& handle, Stats& stats);

}  // namespace hitman2_silent_assassin
