#pragma once

#include "../game_info.hpp"
#include "version.hpp"

namespace hitman_absolution {

GameHookReady hook_ready(Version version);

GameHook hook(Version version);

}  // namespace hitman_absolution