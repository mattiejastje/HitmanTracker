#pragma once

#include "../base_ptrs.hpp"
#include "../hook.hpp"

namespace hitman2_silent_assassin {

HookPtr hook(std::shared_ptr<void> handle, const BasePtrs& base_ptrs);

}