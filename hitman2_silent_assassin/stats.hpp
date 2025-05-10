#pragma once

#include "../mem/handle.hpp"
#include "../module_base.hpp"
#include "../stats.hpp"

namespace hitman2_silent_assassin {

void update_slow(
    void* handle,
    const ModuleBase& module_base,
    int32_t hook_target_ptr,
    Stats& stats
);

void update_fast(
    void* handle,
    const ModuleBase& module_base,
    int32_t hook_target_ptr,
    Stats& stats
);

}  // namespace hitman2_silent_assassin
