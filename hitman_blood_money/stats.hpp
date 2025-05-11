#pragma once

#include "../base_ptrs.hpp"
#include "../mem/handle.hpp"
#include "../stats.hpp"

namespace hitman_blood_money {

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

}  // namespace hitman_blood_money
