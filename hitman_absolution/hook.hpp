#pragma once

#include "../base_ptrs.hpp"
#include "../hook.hpp"

namespace hitman_absolution {

bool hook_ready(void* handle, const BasePtrs& base_ptrs);

HookPtr hook(std::shared_ptr<void> handle, const BasePtrs& base_ptrs);

}  // namespace hitman_absolution