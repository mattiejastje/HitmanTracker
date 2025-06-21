#pragma once

#include "../base_ptrs.hpp"
#include "../hook.hpp"

namespace hitman_codename_47 {

HookPtr hook(std::shared_ptr<void> handle, const BasePtrs& base_ptrs);

}