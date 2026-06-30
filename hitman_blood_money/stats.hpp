#pragma once

#include "../game.hpp"
#include "version.hpp"

namespace hitman_blood_money {

struct Stats {
    float time;
    int map;
    MapStage map_stage;
    int32_t difficulty;
    StatsValue<std::string> rating;
    StatsValue<int32_t> shots_fired;
    StatsValue<int32_t> headshots;
    StatsValue<int32_t> enemies_killed;
    StatsValue<int32_t> enemies_wounded;
    StatsValue<int32_t> innocents_killed;
    StatsValue<int32_t> innocents_wounded;
    StatsValue<int32_t> police_killed;
    StatsValue<int32_t> police_wounded;
    StatsValue<int32_t> frisk_failed;
    StatsValue<int32_t> cover_blown;
    StatsValue<int32_t> bodies_fnd;
    StatsValue<int32_t> target_bodies_fnd;
    StatsValue<int32_t> uncon_bodies_fnd;
    StatsValue<int32_t> witnesses;
    StatsValue<int32_t> on_camera;
    StatsValue<int32_t> cust_weapons_left;
    StatsValue<int32_t> suit_left;
    int32_t shots_hit;
    int32_t accident_kills;
};

GameStatsSlow update_slow(Version version);

GameStatsFast update_fast(Version version);

}  // namespace hitman_blood_money
