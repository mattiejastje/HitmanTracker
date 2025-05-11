#pragma once

#include "../mem/handle.hpp"
#include "../base_ptrs.hpp"
#include "../stats.hpp"

namespace hitman2_silent_assassin {

void update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
);

void update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
);

}  // namespace hitman2_silent_assassin
