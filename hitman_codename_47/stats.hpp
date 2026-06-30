#pragma once

#include "../game.hpp"
#include "version.hpp"

namespace hitman_codename_47 {

struct Stats {
    float time;
    int map;
    MapStage map_stage;
    int32_t difficulty;
};

GameStatsSlow update_slow(Version version);

GameStatsFast update_fast(Version version);

}  // namespace hitman_codename_47
