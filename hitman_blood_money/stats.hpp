#pragma once

#include "../base_ptrs.hpp"
#include "../label_ptrs.hpp"
#include "../mem/handle.hpp"
#include "../stats.hpp"

namespace hitman_blood_money {

void update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
);

void update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
);

}  // namespace hitman_blood_money
