#pragma once

#include "../base_ptrs.hpp"
#include "../game_info.hpp"
#include "../hook.hpp"
#include "version.hpp"

namespace hitman_absolution {

GameHookReady hook_ready(Version version);

GameHook hook(Version version);

}  // namespace hitman_absolution