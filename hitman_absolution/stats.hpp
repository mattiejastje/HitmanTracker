#pragma once

#include <filesystem>

#include "../base_ptrs.hpp"
#include "../game_info.hpp"
#include "../label_ptrs.hpp"
#include "../stats.hpp"

namespace hitman_absolution {

GameStatsSlow update_slow(const settings::HMA& hma);

bool update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
);

}  // namespace hitman_absolution
