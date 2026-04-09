#pragma once

#include "../base_ptrs.hpp"
#include "../label_ptrs.hpp"
#include "../stats.hpp"

namespace hitman_codename_47 {

bool update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats,
    float dt
);

bool update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats,
    float dt
);

}  // namespace hitman_codename_47
