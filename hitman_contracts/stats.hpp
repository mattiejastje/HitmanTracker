#pragma once

#include <filesystem>

#include "../base_ptrs.hpp"
#include "../label_ptrs.hpp"
#include "../stats.hpp"

namespace hitman_contracts {

bool update_slow(
    const std::filesystem::path& exe_path,
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
);

bool update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
);

}  // namespace hitman_contracts
